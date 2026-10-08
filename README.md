# 🏠 Nhà Thông Minh ESP32 + Xiaozhi MCP

Hệ thống nhà thông minh chạy trên **ESP32**, điều khiển thiết bị bằng **nút bấm** và bằng **giọng nói / AI qua Xiaozhi (giao thức MCP)**. Ngoài ra có đèn hiên tự động, cảm biến lửa và giám sát nhiệt độ - độ ẩm với còi cảnh báo.

## ✨ Tính năng

| Nhóm | Mô tả |
|------|-------|
| 💡 Điều khiển thiết bị | Quạt, đèn phòng khách, đèn phòng ngủ, đèn phòng bếp – bật/tắt bằng nút bấm vật lý hoặc qua Xiaozhi |
| 🗣️ Điều khiển bằng AI | Đăng ký các tool MCP để Xiaozhi bật/tắt từng thiết bị, bật/tắt tất cả, và hỏi nhiệt độ - độ ẩm |
| 🚪 Đèn hiên tự động | Cảm biến hồng ngoại (IR) phát hiện người thì bật đèn, tự tắt sau 5 giây nếu không còn người. Có nút bấm để bật/tắt thủ công |
| 🔥 Cảm biến lửa | Phát hiện lửa thì còi kêu liên tục (ưu tiên cao nhất) |
| 🌡️ Giám sát nhiệt độ | DHT11 đọc mỗi 2 giây, cảnh báo bằng còi theo ngưỡng nhiệt độ |
| 🔌 Chạy độc lập | Đèn hiên, cảm biến lửa và còi hoạt động độc lập, không phụ thuộc kết nối MCP |

## 🚨 Logic cảnh báo

Còi hoạt động theo thứ tự ưu tiên: **lửa > nhiệt độ > 45°C > nhiệt độ > 40°C**

| Điều kiện | Hành động |
|-----------|-----------|
| Phát hiện lửa | Còi kêu liên tục |
| Nhiệt độ > 45°C | Tắt toàn bộ thiết bị chính (quạt + 3 đèn), còi kêu liên tục |
| 40°C < Nhiệt độ ≤ 45°C | Còi nháy mỗi 0,5 giây |
| Bình thường | Còi tắt |

## 🧰 Phần cứng

- 1 × ESP32 DevKit (board nhà thông minh)
- 1 × **ESP32-S3** (board Xiaozhi) + micro I2S (ví dụ INMP441) + module khuếch đại loa I2S (ví dụ MAX98357A) + loa
- 1 × cảm biến nhiệt độ - độ ẩm **DHT11**
- 1 × cảm biến lửa (ngõ ra số)
- 1 × cảm biến hồng ngoại IR (đèn hiên)
- 1 × còi (buzzer)
- 5 × relay / module điều khiển: quạt, đèn phòng khách, đèn phòng ngủ, đèn phòng bếp, đèn hiên
- 5 × nút bấm (quạt, 3 đèn trong nhà, đèn hiên)

## 🔌 Sơ đồ chân

### Thiết bị (ngõ ra)

| Thiết bị | GPIO |
|----------|------|
| Quạt | 2 |
| Đèn phòng khách | 4 |
| Đèn phòng ngủ | 18 |
| Đèn phòng bếp | 19 |
| Đèn hiên | 21 |
| Còi | 33 |

### Nút bấm (`INPUT_PULLUP`, nối GND khi nhấn)

| Nút | GPIO |
|-----|------|
| Quạt | 14 |
| Đèn phòng khách | 27 |
| Đèn phòng ngủ | 26 |
| Đèn phòng bếp | 25 |
| Đèn hiên | 16 |

### Cảm biến

| Cảm biến | GPIO | Ghi chú |
|----------|------|---------|
| DHT11 | 32 | Đọc mỗi 2 giây |
| Cảm biến lửa | 17 | Mặc định `LOW` = có lửa. Nếu còi kêu ngược, đổi `FIRE_DETECTED` thành `HIGH` |
| IR (đèn hiên) | 35 | Mặc định `LOW` = có người (`IR_DETECTED`) |

## 🤖 Các tool MCP

| Tool | Chức năng | Tham số |
|------|-----------|---------|
| `fan_control` | Bật/tắt quạt | `state`: `on` / `off` |
| `living_room_lights_control` | Bật/tắt đèn phòng khách | `state`: `on` / `off` |
| `bedroom_lights_control` | Bật/tắt đèn phòng ngủ | `state`: `on` / `off` |
| `kitchen_lights_control` | Bật/tắt đèn phòng bếp | `state`: `on` / `off` |
| `all_devices_control` | Bật/tắt tất cả thiết bị chính | `state`: `on` / `off` |
| `temperature_and_humidity_values` | Trả về nhiệt độ và độ ẩm hiện tại | không có |

Ví dụ câu lệnh với Xiaozhi: *"Bật đèn phòng ngủ"*, *"Tắt tất cả thiết bị"*, *"Nhiệt độ trong phòng là bao nhiêu?"*

## 📦 Thư viện cần cài (Arduino IDE)

Cài qua **Library Manager** (Sketch → Include Library → Manage Libraries):

| Thư viện | Tác giả | Ghi chú |
|----------|---------|---------|
| `xiaozhi-mcp` (cung cấp `WebSocketMCP.h`) | toddpan | Client MCP cho ESP32 |
| `WebSockets` | Markus Sattler | Thư viện phụ thuộc của MCP client |
| `ArduinoJson` | Benoit Blanchon | Xử lý JSON trong các tool MCP |
| `DHT sensor library` + `Adafruit Unified Sensor` | Adafruit | Đọc DHT11 |

`WiFi.h` có sẵn trong core ESP32.

**Board:** cài *esp32 by Espressif Systems* trong Boards Manager, sau đó chọn **ESP32 Dev Module**.

> Nếu không tìm thấy `xiaozhi-mcp` trong Library Manager, hãy tìm theo tên header `WebSocketMCP` hoặc cài thủ công từ GitHub (Sketch → Include Library → Add .ZIP Library).

## 🎙️ Cài đặt Xiaozhi (nạp firmware bằng ESP-IDF)

Hệ thống gồm **2 board ESP32** hoạt động cùng nhau qua máy chủ Xiaozhi:

| Board | Vai trò | Nạp bằng |
|-------|---------|----------|
| **ESP32 Xiaozhi** (có micro + loa) | "Tai và miệng": nghe giọng nói, trả lời | Firmware [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) qua **ESP-IDF** |
| **ESP32 nhà thông minh** (dự án này) | "Tay chân": điều khiển đèn, quạt, đọc cảm biến | File `smart_home_8.ino` qua Arduino IDE |

Cả hai gắn vào **cùng một agent** trên xiaozhi.me. Khi bạn nói, Xiaozhi sẽ gọi các tool MCP mà board nhà thông minh đã đăng ký.

### Bước 1 – Cài ESP-IDF

1. Làm theo [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) để cài ESP-IDF bằng **dòng lệnh** (nên dùng bản mà README của `xiaozhi-esp32` yêu cầu; tại thời điểm viết là ESP-IDF 5.4 trở lên, bạn nên kiểm tra lại trên repo).
2. Không nên cài bằng extension VS Code vì dễ gặp lỗi build.
3. Mỗi lần mở terminal mới, chạy script export của ESP-IDF để dùng được lệnh `idf.py`:

   ```bash
   # Linux / macOS
   . $HOME/esp/esp-idf/export.sh
   # Windows: mở "ESP-IDF PowerShell/CMD" đã cài sẵn
   ```

### Bước 2 – Tải và cấu hình firmware

```bash
git clone https://github.com/78/xiaozhi-esp32.git
cd xiaozhi-esp32

# Chọn chip theo board của bạn (ví dụ ESP32-S3)
idf.py set-target esp32s3

# Mở menu cấu hình
idf.py menuconfig
```

Trong `menuconfig`:
- Vào mục **Xiaozhi Assistant** → chọn **Board Type** đúng với board bạn đang dùng (hoặc loại board tự đấu dây micro + loa).
- Chọn ngôn ngữ mặc định (**Default Language**) nếu muốn.
- Lưu và thoát.

**Với ESP32-S3 tự đấu dây (micro I2S + loa I2S):**

1. Chạy `idf.py set-target esp32s3`.
2. Trong `menuconfig` → **Xiaozhi Assistant** → **Board Type**, chọn loại board DIY dùng ESP32-S3 (thường là `Bread Compact WiFi` hoặc `Bread Compact WiFi + LCD` nếu có màn hình).
3. Mở file `config.h` trong thư mục `main/boards/<tên-board-đã-chọn>/` của repo để xem các chân GPIO của micro, loa, nút bấm, màn hình, rồi **đấu dây đúng theo file đó** (hoặc sửa file cho khớp dây của bạn).
4. Kiểm tra dung lượng Flash và PSRAM của module S3 (ví dụ N16R8) khớp với cấu hình trong `menuconfig`; nếu sai, board có thể không khởi động hoặc báo lỗi PSRAM.

> Tên loại board và sơ đồ chân có thể thay đổi theo phiên bản repo, hãy lấy theo `config.h` của phiên bản bạn clone.

### Bước 3 – Build và nạp

```bash
idf.py build
idf.py -p COMx flash monitor      # Windows
idf.py -p /dev/ttyUSB0 flash monitor   # Linux
```

- Thay `COMx` / `/dev/ttyUSB0` bằng cổng của board.
- Nếu đổi loại board hoặc cấu hình bị lỗi, chạy `idf.py fullclean` rồi build lại.
- Nếu gặp lỗi lạ sau khi đổi partition/bootloader, chạy `idf.py erase-flash` rồi nạp lại.

### Bước 4 – Cấu hình WiFi và liên kết thiết bị

1. Khi khởi động lần đầu, board Xiaozhi sẽ phát WiFi riêng để bạn vào cấu hình WiFi nhà (làm theo hướng dẫn trên màn hình / loa).
2. Sau khi có mạng, thiết bị đọc ra **mã xác thực 6 số**.
3. Vào [xiaozhi.me](https://xiaozhi.me) → **Console** → thêm thiết bị và nhập mã đó để liên kết thiết bị với tài khoản.

### Bước 5 – Tạo agent và lấy MCP Endpoint

1. Trên xiaozhi.me, tạo hoặc chọn **Agent** gắn với thiết bị vừa liên kết.
2. Mở cấu hình agent, tìm mục **MCP Endpoint** và sao chép đường dẫn:

   ```
   wss://api.xiaozhi.me/mcp/?token=XXXXXXXX
   ```

3. Dán vào biến `mcpEndpoint` trong `smart_home_8.ino` (xem phần nạp code bên dưới).

### Bước 6 – Kiểm tra

1. Nạp `smart_home_8.ino` cho board nhà thông minh, mở Serial Monitor (115200 baud).
2. Thấy `[MCP] Đã kết nối tới máy chủ` và `[MCP] Đã đăng ký tất cả tools` là thành công.
3. Trên trang agent, mục MCP sẽ hiện danh sách tool (`fan_control`, `all_devices_control`, ...).
4. Nói với board Xiaozhi: *"Bật đèn phòng khách"*.

> 💡 Giao diện xiaozhi.me và các tên menu có thể thay đổi theo thời gian. Nếu khác với hướng dẫn trên, hãy xem tài liệu trên chính trang đó và README của repo `xiaozhi-esp32`.

## 🚀 Nạp code lên ESP32

1. Clone repo và mở `smart_home_8.ino` bằng Arduino IDE (tên thư mục nên trùng tên file `.ino`).
2. Sửa thông tin cấu hình trong code:

   ```cpp
   const char* ssid        = "TEN_WIFI";
   const char* password    = "MAT_KHAU_WIFI";
   const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=TOKEN_CUA_BAN";
   ```

3. Chọn board **ESP32 Dev Module**, chọn đúng cổng COM, bấm **Upload**.
4. Mở Serial Monitor (**115200 baud**) để xem log: kết nối WiFi, kết nối MCP, đọc cảm biến.

> ⚠️ **Bảo mật:** Không đưa WiFi password và token MCP thật lên GitHub. Hãy để giá trị mẫu trong code khi commit (hoặc tách ra file `secrets.h` và thêm vào `.gitignore`). Nếu token đã lỡ lộ, hãy tạo lại token trên Xiaozhi.

## 🗂️ Cấu trúc dự án

```
.
├── smart_home_8.ino   # Mã nguồn chính
└── README.md
```

## 🛠️ Xử lý sự cố

| Hiện tượng | Cách xử lý |
|------------|------------|
| ESP32 tự khởi động lại sau khi bật | Không kết nối được WiFi sau ~10 giây → kiểm tra SSID/mật khẩu, dùng WiFi 2.4 GHz |
| Xiaozhi không điều khiển được | Xem log `[MCP] Đã kết nối tới máy chủ`; kiểm tra token còn hạn và đúng agent đã liên kết với thiết bị giọng nói |
| Log báo `Mất kết nối với máy chủ MCP` | Kiểm tra mạng, token; thử reset ESP32 |
| Còi kêu liên tục dù không có lửa | Đổi `FIRE_DETECTED` thành `HIGH` |
| Báo `[DHT] Lỗi đọc cảm biến!` | Kiểm tra dây DHT11, nguồn và chân GPIO 32 |
| Nút bấm không phản hồi | Nút phải nối giữa GPIO và GND (đang dùng điện trở kéo lên nội bộ) |

## 🔭 Hướng phát triển

- Thêm màn hình hiển thị nhiệt độ - độ ẩm
- Thêm tool MCP để điều khiển/đọc trạng thái đèn hiên và cảm biến lửa
- Gửi thông báo về điện thoại khi có cảnh báo lửa hoặc nhiệt độ cao

## 📄 Giấy phép

Phát hành theo giấy phép MIT (hoặc cập nhật theo ý bạn).
