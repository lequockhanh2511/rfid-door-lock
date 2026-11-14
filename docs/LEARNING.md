# Ba phần code để học

Codex viết cả ba phần. Tên dưới đây là người học/chịu trách nhiệm giải thích,
không phải danh tính tác giả Git. Dùng chung một firmware để ghép nhất quán.

## Nguyễn Hữu Anh — RC522, danh sách thẻ, EEPROM

Đọc theo thứ tự:
1. hal/src/rc522.c và hal/inc/rc522_protocol.h: SPI thanh ghi, FIFO, WUPA,
   anticollision/select, BCC/CRC_A, cascade UID 4/7/10 byte, HALT.
2. hal/src/eeprom24.c: page64, ACK polling, timeout và đọc lại sau ghi.
3. app/src/config_store.c: serialize160 byte, CRC32, A/B, commit marker, chọn sequence.
4. app/src/rfid_service.c: tra cứu, thêm/xóa slot, xác thực/đổi PIN, phản hồi thống kê.
5. app/src/task_rfid.c: một task sở hữu RC522/cấu hình, lọc giữ thẻ,
   nhận command và giữ lại reply khi queue đầy.
6. tests/test_rc522.c, test_storage.c, test_config.c: cách tạo lỗi và chứng minh xử lý.

Ví dụ cần giải thích: bảng đang đầy, xóa slot02 → slot03 vẫn là03, thêm UID mới
→ slot02. Nếu mất điện trước marker cuối, bản cũ vẫn hợp lệ. Nếu đã ghi marker
nhưng đọc lại bị lỗi, không được khẳng định thao tác chưa lưu; phải reload.
PIN service không tự cấp quyền Admin; nó chỉ xác minh/ghi dữ liệu cho control.

Tự trả lời: Vì sao UID phải so cả length? Vì sao CRC_A khác CRC32? Vì sao ACK
không đủ để báo dữ liệu đã bền? Vì sao lỗi RC522 không được coi là thẻ đã rời?

## Hà Xuân Khải — keypad, PIN, Admin và trạng thái

Đọc hal/src/keypad.c → app/inc/access_control.h → app/src/access_control.c
→ app/src/task_control.c → tests/test_control.c.
Keypad mapping theo hàng: 123A / 456B / 789C / *0#D. Row open-drain, column pull-up.
Chống dội30ms; một lần giữ chỉ sinh một phím; nhiều phím bị chặn đến khi nhả hết.

Máy trạng thái chỉ nhận key, event và thời gian; không tự gọi I2C/SPI. Hàm send
chỉ đưa lệnh vào queue rồi trả ngay. Khi đợi phản hồi, task vẫn kiểm tra hạn đóng.
Một yêu cầu có request_id; chỉ nhận phản hồi trùng ID và loại lệnh.
PIN thu trong input[], chỉ gửi snapshot số lượng dấu *. Đổi PIN cần hai lần giống nhau.
Admin là biến phiên cục bộ; thoát/hết60s phải xóa quyền. Thẻ khi Admin không mở cửa.

Theo dõi ví dụ: IDLE → A → ADMIN_AUTH → # → WAIT_RESULT → ADMIN_MENU;
2 → DELETE_INPUT → 02# → DELETE_CONFIRM → # → WAIT_RESULT → ADMIN_MENU.
D trong WAIT_RESULT không thể thu hồi một giao dịch EEPROM đã bắt đầu.

Tự trả lời: Vì sao không delay5s để giữ khóa mở? Vì sao `(uint32_t)(now-start)`
hoạt động khi tick tràn? Phản hồi auth đến sau D được xử lý thế nào? Nếu OLED
treo thì hàm GPIO khóa được gọi bởi ai? Vì sao mutation lỗi cần đọc lại cấu hình?

## Lê Quốc Khánh — OLED, LED khóa, hiển thị kết quả

Đọc hal/src/ssd1306.c → hal/src/font5x7.inc → hal/src/lock_output.c
→ app/src/task_ui.c → tests/test_display.c.
Framebuffer1024 byte =128 cột×8 page. Mỗi byte chứa8 pixel dọc. Font5x7 có5
cột và1 cột trống; 21 ký tự/hàng. Byte I2C đầu0x00 chỉ lệnh,0x40 chỉ dữ liệu.
Gửi theo khối16 byte để giảm thời gian giữ mutex chung với EEPROM.

`oled_present` nhận snapshot có state/result/card_id/occupied_mask/remaining_ms.
Nó không biết PIN rõ, không sửa danh sách và không quyết định ai được mở cửa.
Danh sách dùng selected_id để kiểm tra bit `(1u << (selected_id-1))` trong mask.
Task UI lấy snapshot mới nhất; nếu OLED lỗi thì thử init lại, không giữ control.

`lock_output_set` đổi GPIO trực tiếp. Người Khải gọi nó từ task_control vì
Khải giữ hạn5s; phần triển khai phần cứng thuộc bạn. Không đưa thao tác đóng
khóa qua queue OLED: bus lỗi sẽ không được phép làm khóa giữ mở vô hạn.

Tự trả lời: Vì sao byte điều khiển I2C không nằm trong framebuffer? Vì sao có
mailbox dài1 và overwrite? Vì sao LED xanh/đỏ dùng BSRR? Vì sao SPI PA5 không
được dùng làm LED khóa dù board có LD2 ở đó? OLED đổi chữ không dấu vì đâu?

## Cả ba cùng học ghép hệ thống

Đọc app/src/main.c, app_ipc.c, bsp/src/board.c và ports_stm32.c sau ba phần trên.
Luồng thẻ: task_rfid → event queue → control → command queue → service
→ event queue → control đổi LED và gửi snapshot → UI.
Queue copy dữ liệu, không gửi con trỏ stack; EEPROM/OLED dùng chung mutex I2C.
Service là nơi duy nhất sửa dữ liệu; control là nơi duy nhất cấp quyền và đổi khóa.

Chạy host tests, đọc từng assert gắn với SPEC. PASS trên host không chứng minh
scheduler, dây nối, RF, thời gian hay nguồn trên kit. Không học hàng nghìn dòng
middleware trước; bắt đầu bằng code của nhóm trong app/, hal/, bsp/.
