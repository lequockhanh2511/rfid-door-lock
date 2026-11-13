# Kế hoạch kiểm chứng

| Nhóm | Tiêu chí | Trạng thái M1 |
|---|---|---|
| Header | Từng header độc lập C11, warning thành lỗi; dùng chung không xung đột | chạy check_contracts.py |
| Card store | Trùng, đầy 10, xóa 02 không dồn số, tái dùng 02 | chưa triển khai |
| PIN/Admin | Sai không mở/sửa; hủy/timeout xóa phiên; đổi PIN hai lần | chưa triển khai |
| Thời gian | Tự đóng 5 giây, gia hạn, rollover uint32, OLED lỗi không ảnh hưởng | chưa triển khai |
| Storage | Mất điện từng bước ghi A/B; khôi phục đúng; không seed khi boot thường | chưa triển khai |
| IPC | Queue đầy; response cũ; timeout mutation; UI mailbox | chưa triển khai |
| ARM | Build/link đúng STM32F411, memory map/stack config | thiếu toolchain/SDK |
| Kit | SPI/I2C, keypad debounce, LED/OLED, UID thực, EEPROM | chưa có kit |
| RTOS | Stack watermark, heap, scheduling, ISR priority | chưa có kit |

Không tính compile check là test thuật toán. Không gọi host test là mô phỏng tín hiệu điện.
