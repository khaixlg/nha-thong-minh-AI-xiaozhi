#include <WiFi.h>
#include <WebSocketMCP.h>
#include <DHT.h>

// --- Pin thiết bị chính ---
#define FAN    2
#define LIGHT1 4
#define LIGHT2 18
#define LIGHT3 19

// --- Pin nút bấm chính (INPUT_PULLUP, GND khi nhấn) ---
#define BTN_FAN    14
#define BTN_LIGHT1 27
#define BTN_LIGHT2 26
#define BTN_LIGHT3 25

// --- Đèn hiên (độc lập, không qua MCP) ---
#define LIGHT_PORCH    21
#define BTN_PORCH      16
#define IR_SENSOR      35
#define IR_DETECTED    LOW
#define PORCH_AUTO_OFF 5000

// --- Cảm biến lửa (độc lập, không qua MCP) ---
#define FIRE_SENSOR   17
#define FIRE_DETECTED LOW  // đổi thành HIGH nếu còi kêu ngược

// --- Pin DHT11 & Còi ---
#define DHT_PIN  32
#define DHT_TYPE DHT11
#define BUZZER   33

// --- WiFi ---
const char* ssid     = "kh";
const char* password = "88888888";

// --- MCP ---
const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VySWQiOjg3Mjc5MCwiYWdlbnRJZCI6MTYzMzIyMSwiZW5kcG9pbnRJZCI6ImFnZW50XzE2MzMyMjEiLCJwdXJwb3NlIjoibWNwLWVuZHBvaW50IiwiaWF0IjoxNzc0NzExNzAzLCJleHAiOjE4MDYyNjkzMDN9.jqd-U5rOY8lt11ANn3nf7eDOOPxf2m1yTV_QGH1GybzZdJ9GR-ZW2_z6kt-nBCX1csJJFXyGmyELVTwtXG6Asw";

WebSocketMCP mcpClient;
DHT dht(DHT_PIN, DHT_TYPE);

// --- Biến cảm biến nhiệt độ ---
float temperature      = 0;
float humidity         = 0;
unsigned long lastDHTRead      = 0;
unsigned long lastBuzzerToggle = 0;
bool alertMode = false;  // cảnh báo nhiệt độ > 40°C
#define DHT_INTERVAL 2000

// --- Biến đèn hiên ---
bool porchState        = false;
bool porchManual       = false;
int  lastBtnPorch      = HIGH;
unsigned long lastIRDetect = 0;

// --- Biến cảm biến lửa ---
bool fireDetected = false;

// -------------------------------------------------------
// Còi — ưu tiên: lửa > nhiệt độ > 45 > nhiệt độ > 40
// -------------------------------------------------------
void updateBuzzer() {
    if (fireDetected) {
        // Lửa: ưu tiên cao nhất — còi kêu liên tục
        digitalWrite(BUZZER, HIGH);
    } else if (temperature > 45) {
        // Nhiệt độ cực cao — còi kêu liên tục
        digitalWrite(BUZZER, HIGH);
    } else if (alertMode) {
        // Nhiệt độ > 40 — còi nháy 0.5s
        if (millis() - lastBuzzerToggle >= 500) {
            lastBuzzerToggle = millis();
            digitalWrite(BUZZER, !digitalRead(BUZZER));
        }
    } else {
        digitalWrite(BUZZER, LOW);
    }
}

// -------------------------------------------------------
// Cảm biến lửa (độc lập)
// -------------------------------------------------------
void handleFire() {
    int reading = digitalRead(FIRE_SENSOR);

    if (reading == FIRE_DETECTED) {
        if (!fireDetected) {
            fireDetected = true;
            Serial.println("[FIRE] Phát hiện lửa! Còi báo động!");
        }
    } else {
        if (fireDetected) {
            fireDetected = false;
            Serial.println("[FIRE] Hết lửa. Tắt còi.");
        }
    }
}

// -------------------------------------------------------
// Quản lý trạng thái & nút bấm thiết bị chính
// -------------------------------------------------------
struct Device {
    const char*   name;
    int           devicePin;
    int           btnPin;
    bool          state;
    int           lastBtnState;
    unsigned long lastDebounce;
};

Device devices[] = {
    { "Quạt",            FAN,    BTN_FAN,    false, HIGH, 0 },
    { "Đèn phòng khách", LIGHT1, BTN_LIGHT1, false, HIGH, 0 },
    { "Đèn phòng ngủ",   LIGHT2, BTN_LIGHT2, false, HIGH, 0 },
    { "Đèn phòng bếp",   LIGHT3, BTN_LIGHT3, false, HIGH, 0 },
};
const int DEVICE_COUNT = sizeof(devices) / sizeof(devices[0]);

void applyState(int index) {
    digitalWrite(devices[index].devicePin, devices[index].state ? HIGH : LOW);
    Serial.printf("[BTN] %s: %s\n", devices[index].name, devices[index].state ? "BẬT" : "TẮT");
}

void turnOffAllDevices() {
    for (int i = 0; i < DEVICE_COUNT; i++) {
        devices[i].state = false;
        digitalWrite(devices[i].devicePin, LOW);
    }
    Serial.println("[ALERT] Đã tắt tất cả thiết bị");
}

// -------------------------------------------------------
// Nút bấm thiết bị chính
// -------------------------------------------------------
void handleButtons() {
    for (int i = 0; i < DEVICE_COUNT; i++) {
        int reading = digitalRead(devices[i].btnPin);

        if (reading == LOW && devices[i].lastBtnState == HIGH) {
            delay(50);
            if (digitalRead(devices[i].btnPin) == LOW) {
                devices[i].state = !devices[i].state;
                applyState(i);
                while (digitalRead(devices[i].btnPin) == LOW) delay(10);
                delay(50);
            }
        }

        devices[i].lastBtnState = reading;
    }
}

// -------------------------------------------------------
// Đèn hiên — nút bấm + cảm biến IR (độc lập với MCP)
// -------------------------------------------------------
void setPorchLight(bool on, const char* reason) {
    porchState = on;
    digitalWrite(LIGHT_PORCH, on ? HIGH : LOW);
    Serial.printf("[HIÊN] %s (%s)\n", on ? "BẬT" : "TẮT", reason);
}

void handlePorch() {
    int btnReading = digitalRead(BTN_PORCH);
    if (btnReading == LOW && lastBtnPorch == HIGH) {
        delay(50);
        if (digitalRead(BTN_PORCH) == LOW) {
            porchManual = !porchState;
            setPorchLight(!porchState, "nút bấm");
            while (digitalRead(BTN_PORCH) == LOW) delay(10);
            delay(50);
        }
    }
    lastBtnPorch = btnReading;

    int ir = digitalRead(IR_SENSOR);
    if (ir == IR_DETECTED) {
        lastIRDetect = millis();
        if (!porchState) {
            porchManual = false;
            setPorchLight(true, "IR phát hiện người");
        }
    } else {
        if (!porchManual && porchState) {
            if (millis() - lastIRDetect >= PORCH_AUTO_OFF) {
                setPorchLight(false, "auto-off sau 5s");
            }
        }
    }
}

// -------------------------------------------------------
// DHT11
// -------------------------------------------------------
void handleDHT() {
    if (millis() - lastDHTRead < DHT_INTERVAL) return;
    lastDHTRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
        Serial.println("[DHT] Lỗi đọc cảm biến!");
        return;
    }

    temperature = t;
    humidity    = h;
    Serial.printf("[DHT] Nhiệt độ: %.1f°C  Độ ẩm: %.1f%%\n", temperature, humidity);

    if (temperature > 45) {
        turnOffAllDevices();
        alertMode = false;
        Serial.println("[ALERT] Nhiệt độ > 45°C — còi liên tục!");

    } else if (temperature > 40) {
        alertMode = true;
        Serial.println("[ALERT] Nhiệt độ > 40°C — còi nháy liên tục!");

    } else {
        alertMode = false;
    }
}

// -------------------------------------------------------
// MCP
// -------------------------------------------------------
const char* STATE_SCHEMA = R"({
    "type": "object",
    "properties": {
        "state": {
            "type": "string",
            "enum": ["on", "off"]
        }
    },
    "required": ["state"]
})";

const char* SENSOR_SCHEMA = R"({
    "type": "object",
    "properties": {}
})";

WebSocketMCP::ToolResponse handlePinControl(const String& args, int* deviceIndices, int count) {
    StaticJsonDocument<256> doc;

    if (deserializeJson(doc, args) != DeserializationError::Ok) {
        return WebSocketMCP::ToolResponse("{\"success\":false,\"error\":\"invalid json\"}");
    }

    String state = doc["state"].as<String>();
    bool newState;

    if      (state == "on")  newState = true;
    else if (state == "off") newState = false;
    else return WebSocketMCP::ToolResponse("{\"success\":false,\"error\":\"invalid state\"}");

    for (int i = 0; i < count; i++) {
        devices[deviceIndices[i]].state = newState;
        applyState(deviceIndices[i]);
    }

    return WebSocketMCP::ToolResponse("{\"success\":true,\"state\":\"" + state + "\"}");
}

void registerMcpTools() {

    static int idxFan[]    = { 0 };
    static int idxLight1[] = { 1 };
    static int idxLight2[] = { 2 };
    static int idxLight3[] = { 3 };
    static int idxAll[]    = { 0, 1, 2, 3 };

    mcpClient.registerTool("fan_control",
        "Bật hoặc tắt quạt. Chỉ dùng khi người dùng đề cập riêng đến quạt.",
        STATE_SCHEMA,
        [](const String& args) { return handlePinControl(args, idxFan, 1); });

    mcpClient.registerTool("living_room_lights_control",
        "Bật hoặc tắt đèn phòng khách. Chỉ dùng khi người dùng đề cập riêng đến đèn phòng khách.",
        STATE_SCHEMA,
        [](const String& args) { return handlePinControl(args, idxLight1, 1); });

    mcpClient.registerTool("bedroom_lights_control",
        "Bật hoặc tắt đèn phòng ngủ. Chỉ dùng khi người dùng đề cập riêng đến đèn phòng ngủ.",
        STATE_SCHEMA,
        [](const String& args) { return handlePinControl(args, idxLight2, 1); });

    mcpClient.registerTool("kitchen_lights_control",
        "Bật hoặc tắt đèn phòng bếp. Chỉ dùng khi người dùng đề cập riêng đến đèn phòng bếp.",
        STATE_SCHEMA,
        [](const String& args) { return handlePinControl(args, idxLight3, 1); });

    mcpClient.registerTool("all_devices_control",
        "Bật hoặc tắt TẤT CẢ thiết bị cùng lúc. Dùng tool này thay vì gọi từng tool riêng lẻ khi người dùng muốn bật tất cả hoặc tắt tất cả.",
        STATE_SCHEMA,
        [](const String& args) { return handlePinControl(args, idxAll, 4); });

    mcpClient.registerTool(
        "temperature_and_humidity_values",
        "Lấy nhiệt độ và độ ẩm từ cảm biến. Gọi tool này khi người dùng hỏi về nhiệt độ hoặc độ ẩm trong phòng.",
        SENSOR_SCHEMA,
        [](const String& args) {
            Serial.println("[MCP] Xiaozhi hỏi cảm biến");
            Serial.printf("  Nhiệt độ: %.1f°C\n", temperature);
            Serial.printf("  Độ ẩm:    %.1f%%\n", humidity);

            StaticJsonDocument<128> doc;
            doc["temperature"] = temperature;
            doc["humidity"]    = humidity;
            String json;
            serializeJson(doc, json);
            return WebSocketMCP::ToolResponse(json);
        }
    );

    Serial.println("[MCP] Đã đăng ký tất cả tools");
}

void onConnectionStatus(bool connected) {
    if (connected) {
        Serial.println("[MCP] Đã kết nối tới máy chủ");
        registerMcpTools();
    } else {
        Serial.println("[MCP] Mất kết nối với máy chủ MCP");
    }
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {
    Serial.begin(115200);

    for (int i = 0; i < DEVICE_COUNT; i++) {
        pinMode(devices[i].devicePin, OUTPUT);
        digitalWrite(devices[i].devicePin, LOW);
        pinMode(devices[i].btnPin, INPUT_PULLUP);
    }

    pinMode(LIGHT_PORCH, OUTPUT);
    digitalWrite(LIGHT_PORCH, LOW);
    pinMode(BTN_PORCH, INPUT_PULLUP);
    pinMode(IR_SENSOR, INPUT);

    pinMode(FIRE_SENSOR, INPUT);

    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);

    dht.begin();

    Serial.print("Đang kết nối WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nKết nối WiFi thất bại! Khởi động lại...");
        ESP.restart();
    }

    Serial.println("\nWiFi đã kết nối");
    Serial.println("IP thiết bị: " + WiFi.localIP().toString());

    mcpClient.begin(mcpEndpoint, onConnectionStatus);
}

// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {
    handleFire();    // cảm biến lửa — ưu tiên đọc trước
    handleButtons();
    handlePorch();
    handleDHT();
    updateBuzzer();  // còi tổng hợp: lửa > t°>45 > t°>40
    mcpClient.loop();
}
