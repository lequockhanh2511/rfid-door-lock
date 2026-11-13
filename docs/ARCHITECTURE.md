# Kiến trúc và hợp đồng v1

| Chủ phần | Thành phần | Quyền sở hữu |
|---|---|---|
| Anh | task_rfid, RC522, EEPROM, card_store | SPI và cấu hình bền vững |
| Khải | task_control, keypad, PIN/Admin | Máy trạng thái, xác thực phiên, hạn đóng |
| Khánh | task_ui, OLED, lock_output | UI và triển khai GPIO đầu ra |

## Luồng quẹt thẻ
RFID phát CARD_PRESENT (UID), control xét chế độ. IDLE/PIN_ENTRY/UNLOCKED gửi
LOOKUP_CARD; ADD_WAIT_CARD gửi ADD_CARD; Admin khác bỏ qua thẻ.
RFID trả COMMAND_RESULT có request_id; control chỉ xử lý kết quả đúng yêu cầu
và ngữ cảnh còn hiệu lực. Lookup thành công mở 5 giây bằng lock_output_set,
đẩy snapshot UI; hết hạn control đóng GPIO ngay, không chờ OLED.

## PIN và quyền sửa
Control thu 6 ký tự, gửi VERIFY_PIN với role. RFID so sánh cấu hình trong RAM,
trả OK hoặc DENIED; control lưu quyền Admin trong phiên, xóa khi thoát/timeout.
Chỉ control phát ADD/DELETE/SET_PIN sau khi phiên Admin hợp lệ. API nội bộ
không phải giao diện từ mạng/UART. SET_PIN chỉ sau xác nhận hai lần ở control.
Không đưa PIN vào UI/log; xóa các bản sao trong bộ đệm/queue sau khi dùng.

## Tránh race
Control cấp request_id khác 0 và không tái dùng ID còn có thể nhận kết quả cũ.
Một yêu cầu đang xử lý; các sự kiện đến khi bận báo BUSY/không mở từ kết quả cũ.
Timeout/hủy không hủy được giao dịch EEPROM đã commit. Không nhận thao tác sửa
mới cho đến khi giao dịch trước được giải quyết hoặc hệ thống vào lỗi cần phục hồi.
Khi quay lại menu, GET_STATUS đồng bộ revision/occupied_mask.

## Queue và mutex
Thông điệp copy theo giá trị; không con trỏ bên trong. Queue nghiệp vụ dùng
send/receive có timeout hữu hạn. Mất result do queue đầy phải báo lỗi/counter,
control hết hạn xử lý lỗi, không mở. UI mailbox dài 1 được overwrite.
I2C mutex duy nhất dùng bởi OLED và EEPROM. Không giữ I2C khi chờ EEPROM ghi nội.
Không chờ phản hồi task khác trong khi đang giữ mutex.

## Kết quả và tính bền vững
APP_OK chỉ cho mutation sau lưu và verify. Thất bại giữ bản RAM đã commit cũ;
nếu trạng thái commit không rõ thì reload trước mutation tiếp theo.
config_revision là sequence bản lưu, không phải request_id.
Không ép struct trực tiếp thành bytes EEPROM; cần định dạng serialize có version.

## Startup cần triển khai M4
BSP/clock -> GPIO đóng -> bus -> đọc cấu hình -> tạo IPC -> tạo task -> scheduler.
Nếu lỗi tạo queue/task hoặc cấu hình không hợp lệ, giữ đóng và báo lỗi phù hợp.
M1 chưa lựa chọn FreeRTOS tick rate, heap bytes hay pin map. Số stack trong cfg
là WORDS và chỉ là dự kiến. Đo high-water mark/heap trên kit trước release.
