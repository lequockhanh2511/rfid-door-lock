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

## Cập nhật M3 — 14/11/2026

Host: tests/README.md và reports/m3-host-tests.txt ghi test đã chạy.
ARM: reports/m3-arm-app.txt, m3-arm-diagnostic.txt, m3-arm-provision-*.txt.
Provision compile dùng UID tổng hợp chỉ để kiểm tra build; image không được giao
và không hề nạp kit. Hai UID thật và kết quả vật lý chưa có.

Checklist cần thực hiện trên kit trước release:
- [ ] Nguồn/chân3,3V, pull-up I2C, RC522 version, OLED address, EEPROM WP.
- [ ] Diagnostic đọc hai UID thật, keypad đủ16 phím, OLED rõ chữ, LED đúng chiều.
- [ ] Provision có chủ đích và từ chối khi cấu hình hợp lệ đã tồn tại.
- [ ] Demo10 slot; đúng/sai/trùng/đầy, xóa02 không dồn số, thêm lại02.
- [ ] PIN/Admin đúng/sai/đổi/hủy/timeout, card trong Admin không mở, giữ thẻ không lặp.
- [ ] Đo tự đóng5s khi OLED hoặc EEPROM lỗi; quan sát jitter thay vì ghi 'chính xác' từ code.
- [ ] Cắt nguồn thực lúc ghi nhiều lần; boot luôn dùng bản hợp lệ hoặc khóa an toàn.
- [ ] Request đồng thời, queuepressure, retry reader/display, phản hồi muộn.
- [ ] Đo control/rfid/ui stack còn lại và heap; kiểm tra stackoverflow/malloc hook.
- [ ] Kiểm tra USB/ST-LINK khi dùng Ubuntu VirtualBox; lưu phiên bản và log nạp.
- [ ] Ba thành viên review, ghi bằng chứng rồi mới merge main/tag v1.0.0.
