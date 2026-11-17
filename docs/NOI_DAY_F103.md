# Nối dây dự kiến – STM32F103C8T6 (Blue Pill)

Đây là bảng chân được chọn trong BSP mới, chưa xác nhận trên kit. Kiểm tra ký hiệu silkscreen và module thật trước khi nối. Tắt nguồn trước khi lắp. Tất cả GND phải chung.

| Module | Chân module | STM32 / nguồn |
|---|---|---|
| RC522 | SDA/SS | PA4 |
| RC522 | SCK | PA5 |
| RC522 | MISO | PA6 |
| RC522 | MOSI | PA7 |
| RC522 | RST | PB0 |
| RC522 | 3.3V / GND | 3.3V / GND |
| RC522 | IRQ | Không nối, poll qua SPI |
| OLED SSD1306 | SCL | PB6 (I2C1 mặc định) |
| OLED SSD1306 | SDA | PB7 |
| OLED SSD1306 | VCC / GND | 3.3V / GND nếu module hỗ trợ 3.3V |
| Keypad | R1, R2, R3, R4 | PB12, PB13, PB14, PB15 |
| Keypad | C1, C2, C3, C4 | PB5, PB8, PB9, PB10 |
| Servo SG90 | Signal | PA0 (TIM2_CH1) |
| Servo SG90 | VCC | Nguồn 5V ngoài ổn định |
| Servo SG90 | GND | GND nguồn 5V và GND STM32 |
| ST-LINK | SWDIO; SWCLK; GND | PA13; PA14; GND |

Không có EEPROM ngoài, không dùng cặp LED đỏ/xanh. Bàn phím ma trận là linh kiện thụ động, cần xác định hàng/cột theo dây thực tế. OLED I2C có pull-up về 3.3V, địa chỉ mặc định 0x3C. Không đưa 5V lên RC522 hoặc I2C. Dùng nguồn 5V riêng có đủ dòng cho SG90 (ví dụ 5V 2A), tránh cấp từ chân 3.3V hoặc GPIO; tránh nối chập hai nguồn 5V. Nếu mức tín hiệu 3.3V của PA0 không tương thích module servo thực tế, dùng chuyển mức phù hợp.

PWM: timer clock danh định 8 MHz, PSC=7 → 1 MHz, ARR=19999 → chu kỳ 20 ms (50 Hz). Compare 1000 µs đóng / 1500 µs mở là giá trị ban đầu, không phải góc tuyệt đối đã đo. Tháo chốt khi hiệu chỉnh, không cố ép servo vào giới hạn cơ khí. Sửa `APP_SERVO_CLOSED_US`/`APP_SERVO_OPEN_US` trong `app/inc/app_cfg.h` rồi build lại nếu cần. Khởi động sẽ phát lệnh đóng. Không có phản hồi vị trí.

## Flash nội
Trang A 0x0800F800–0x0800FBFF; trang B 0x0800FC00–0x0800FFFF. Linker dành riêng 2 trang cuối của Flash 64 KiB. Khi nạp lại app, chỉ xóa các trang thuộc firmware; mass erase sẽ mất cấu hình. Cần bật BOOT0=0 để khởi động từ Flash sau khi nạp.
