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

- 1 × ESP32 DevKit
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

- **WebSocketMCP** (client MCP cho ESP32)
- **DHT sensor library** (Adafruit) và **Adafruit Unified Sensor**
- **ArduinoJson**
- `WiFi.h` (có sẵn trong core ESP32)

Board: `ESP32 Dev Module` (cài ESP32 board package của Espressif).

## 🚀 Cài đặt & nạp code

1. Clone repo và mở file `smart_home_8.ino` bằng Arduino IDE.
2. Lấy **MCP endpoint** từ [xiaozhi.me](https://xiaozhi.me) (mục điểm cuối MCP của agent).
3. Sửa thông tin cấu hình trong code:

   ```cpp
   const char* ssid        = "TEN_WIFI";
   const char* password    = "MAT_KHAU_WIFI";
   const char* mcpEndpoint = "wss://api.xiaozhi.me/mcp/?token=TOKEN_CUA_BAN";
   ```

4. Chọn board **ESP32 Dev Module**, chọn cổng COM rồi nạp code.
5. Mở Serial Monitor (**115200 baud**) để xem log: kết nối WiFi, kết nối MCP, đọc cảm biến.

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
| Xiaozhi không điều khiển được | Xem log `[MCP] Đã kết nối tới máy chủ`; kiểm tra token còn hạn |
| Còi kêu liên tục dù không có lửa | Đổi `FIRE_DETECTED` thành `HIGH` |
| Báo `[DHT] Lỗi đọc cảm biến!` | Kiểm tra dây DHT11, nguồn và chân GPIO 32 |
| Nút bấm không phản hồi | Nút phải nối giữa GPIO và GND (đang dùng điện trở kéo lên nội bộ) |

## 🔭 Hướng phát triển

- Thêm màn hình hiển thị nhiệt độ - độ ẩm
- Thêm tool MCP để điều khiển/đọc trạng thái đèn hiên và cảm biến lửa
- Gửi thông báo về điện thoại khi có cảnh báo lửa hoặc nhiệt độ cao

## 📄 Giấy phép

Phát hành theo giấy phép MIT (hoặc cập nhật theo ý bạn).
