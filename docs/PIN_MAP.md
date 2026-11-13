# Pin map M2 — NUCLEO-F411RE / MB1136

Đối chiếu UM1724 Rev17, bảng 29 trang 59 và DS10314 Rev8 bảng alternate functions.
Đây là cấu hình firmware đã chọn; chưa xác nhận revision và nối dây của kit thực.

| Tín hiệu | MCU | Header |
|---|---|---|
| RC522 SCK | PA5 AF5 SPI1 | CN10-11 / D13 |
| RC522 MISO | PA6 AF5 | CN10-13 / D12 |
| RC522 MOSI | PA7 AF5 | CN10-15 / D11 |
| RC522 SDA/SS (không phải I2C SDA) | PA4 output | CN7-32 |
| RC522 RST | PB0 output | CN7-34 |
| OLED + EEPROM SCL | PB8 AF4 I2C1 | CN10-3 / D15 |
| OLED + EEPROM SDA | PB9 AF4 I2C1 | CN10-5 / D14 |
| Keypad R1,R2,R3,R4 | PC4,PC5,PC6,PC7 | CN10-34,6,4,19 |
| Keypad C1,C2,C3,C4 | PC8,PC9,PC10,PC11 | CN10-2,1; CN7-1,2 |
| LED đỏ (qua 330 ohm tới A, K về GND) | PB1 | CN10-24 |
| LED xanh (qua 330 ohm tới A, K về GND) | PB10 | CN10-25 |
| Nguồn module | 3V3 | CN7-16 |
| GND chung | GND | CN7-20 |

RC522 IRQ để trống. EEPROM A0/A1/A2 nối GND -> địa chỉ 7-bit 0x50; WP nối GND.
OLED mặc định 0x3C, cấu hình 0x3D nếu module thực dùng địa chỉ khác.
Cần pull-up SDA/SCL về 3,3 V (kiểm tra module đã có trước khi thêm); không kéo về 5 V.
Keypad thứ tự dây thực phải dò R/C, không suy ra từ vị trí cáp. Row open-drain,
không chọn row = thả nổi; columns dùng pull-up. Không short hai output push-pull.
PA5 chia sẻ LED LD2 onboard: LED onboard có thể nháy theo SPI, KHÔNG dùng làm LED khóa.
PA13/PA14 giữ cho SWD; PA2/PA3 giữ cho ST-LINK VCP (UART chưa triển khai M2).
Không đổi jumper cấp nguồn; kit dự kiến cấp USB ST-LINK. Cần đối chiếu board thật trước cấp điện.
HSI 16 MHz, SPI1 /16 = 1 MHz, I2C1 100 kHz. Không phụ thuộc HSE/ST-LINK MCO.
