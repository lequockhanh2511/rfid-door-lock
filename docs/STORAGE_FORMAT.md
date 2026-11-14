# EEPROM và bản ghi A/B

24LC256: bank A bắt đầu 0x0000, bank B 0x0100; mỗi bản dài 160 byte.
Khoảng trống đến 0x00FF/0x01FF không sử dụng. Code không ghi struct thẳng ra EEPROM.
Số nhiều byte là little-endian. CRC32 dùng đa thức phản xạ 0xEDB88320,
khởi tạo 0xFFFFFFFF, đảo bit kết quả cuối.

| Offset trong bank | Số byte | Nội dung |
|---|---:|---|
| 0 | 4 | ASCII RF3! |
| 4 | 1 | Version 1 |
| 5 | 3 | Reserved = 0 |
| 8 | 4 | Sequence modulo 2^32 |
| 12 | 120 | 10 slot × 12 byte: used, UID length, UID[10] |
| 132 | 6 | User PIN (ASCII, không lưu NUL) |
| 138 | 6 | Admin PIN |
| 144 | 11 | Reserved = 0 |
| 155 | 4 | CRC32 của byte 0..154 |
| 159 | 1 | Commit marker 0xA5 |

Slot trống serialize thành 12 byte 0. UID hợp lệ dài 4/7/10 byte, không trùng
với slot đang dùng khác. PIN đúng sáu chữ số. Khi load, xác minh magic, version,
reserved, CRC, marker, UID/PIN và tính duy nhất. Cả hai hợp lệ: chọn sequence mới
hơn bằng phép trừ unsigned (khoảng cách dưới 2^31; mỗi lần lưu tăng đúng 1).

Ghi vào bank đối diện: marker=0 → payload+CRC → đọc/so sánh → marker=0xA5
→ đọc/so sánh toàn bản. Chỉ sau đó trả OK và thay cấu hình RAM. Driver cấp dưới
chia page64, ACK polling, đọc lại từng page. Bank đang dùng không bị sửa.
Nếu bất kỳ bước ghi/đọc lại thất bại, store.ready=false; không cho tra cứu/PIN
hay sửa tiếp đến khi GET_STATUS load lại. Việc đã commit nhưng đọc lại lỗi
có thể khiến lần reload thấy bản mới, nên UI không khẳng định đã hoàn tác.

Nếu một bank sai checksum, còn bank kia hợp lệ: dùng bank hợp lệ.
Nếu giao dịch đọc một bank lỗi: báo IO_ERROR, không đoán bank nào mới hơn.
Nếu cả hai không hợp lệ: SETUP_REQUIRED, không khôi phục PIN demo tự động.
Provision là firmware riêng, từ chối ghi nếu đã có cấu hình hợp lệ.

Host tests: cắt sau mỗi byte tại 162 vị trí cho 3 trường hợp (khởi tạo,
ghi sang bank trống, ghi đè bank cũ); lật từng bit của bản mới 1280 trường hợp;
lỗi bus, lỗi đọc sau commit, giữ slot khi xóa, sequence wrap. Đây là mô hình
EEPROM byte, chưa mô phỏng sụt áp/điện áp ghi thật hoặc thời gian analog.

PIN lưu dạng rõ phục vụ bài học, UID có thể sao chép; CRC không phải cơ chế
chống sửa ác ý. Không đưa PIN vào log/OLED, nhưng bản ghi EEPROM và RAM/queue
vẫn có PIN. Chưa có mã hóa, rate-limit thử PIN hoặc cơ chế bảo vệ sản phẩm thực.
