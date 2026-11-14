# Thao tác và demo ứng dụng

Hành vi dưới đây đã kiểm thử bằng logic C trên host, chưa xác nhận trên kit.
Đầu ra khóa hiện là LED: đỏ = đóng, xanh = mở, không có servo/relay.
OLED dùng chữ không dấu. Sau provision demo: PIN người dùng `123456`, Admin `654321`.
Không có PIN dự phòng khi EEPROM trống/hỏng.

| Thao tác | Cách dùng |
|---|---|
| Mở bằng RFID | Quẹt một thẻ đã lưu; màn hình hiện CARD:01..10, mở 5 giây |
| Mở bằng PIN | Nhập đủ 6 số rồi #; * xóa số cuối, D hủy |
| Vào Admin | Khi đang đóng: A → 654321 → # |
| Thêm thẻ | Trong menu: 1 → quẹt thẻ; dùng slot trống thấp nhất |
| Xóa slot 02 | Trong menu: 2 → 02 → # → kiểm tra số → # lần nữa |
| Xem danh sách | 3 → 4/6 lùi/tiến; USED/EMPTY; D về menu |
| Đổi PIN mở | 4 → PIN mới → # → nhập lại giống hệt → # |
| Đổi PIN Admin | 5 → PIN mới → # → nhập lại giống hệt → # |
| Thoát | D về menu, D tại menu thoát Admin |
| Lỗi/SETUP REQUIRED | D đọc lại cấu hình; không tự khôi phục PIN mặc định |

Thẻ trong Admin không mở khóa. Chỉ mục Thêm thẻ sử dụng thẻ để đăng ký.
Không vào Admin khi khóa đang mở. Quẹt thẻ hợp lệ mới lúc mở gia hạn thêm 5 giây;
giữ nguyên thẻ trước đầu đọc không gia hạn: cần bỏ thẻ ra (ít nhất ba lần
poll báo không có thẻ, thường trên 225 ms) rồi đưa lại. Không quẹt nhiều thẻ cùng lúc.
Nhập không thao tác 20 giây sẽ hủy; phiên Admin không hoạt động 60 giây sẽ thoát.
Có thể quẹt thẻ hợp lệ khi đang nhập PIN mở cửa; bộ đệm PIN được xóa.

Khi PLEASE WAIT, D đánh dấu hủy ngữ cảnh nhưng vẫn đợi phản hồi hoặc timeout.
Nếu lệnh xóa/thêm/đổi PIN đã được gửi, nó có thể vẫn được lưu; D không hoàn tác
EEPROM. Khi phản hồi đến, quyền Admin bị xóa và số thẻ được đồng bộ.
Nếu không nhận phản hồi sau 3 giây: khóa đóng, ERROR; D yêu cầu đọc lại cấu hình.
Sau lỗi ghi, kiểm tra lại danh sách/PIN vì bản mới có thể đã commit trước lỗi đọc.

Thông báo `FULL - DELETE A SLOT`: đã đủ 10 thẻ, xóa một slot trước.
`CARD ALREADY SAVED`: UID trùng, không ghi lại. `QUEUE BUSY - RETRY`: chưa gửi lệnh,
thử lại; không có thông báo thành công giả. Thẻ quẹt khi bận có thể bị bỏ qua;
rời thẻ rồi quẹt lại khi màn hình sẵn sàng. Sự kiện trùng mili giây với lúc
đổi trạng thái cũng bị bỏ qua vì không thể xác định thứ tự chắc chắn.

## Kịch bản demo đủ 10 thẻ

1. Khởi động sau provision: TOTAL:10, slot 01/02 là hai UID thật, 03..10 là UID ảo.
2. Quẹt thẻ thật 01: hiện CARD:01, LED xanh; sau 5 giây LED đỏ.
3. Quẹt thẻ vật lý thứ ba chưa đăng ký: CARD NOT FOUND, không mở.
4. Nhập 123456#: mở; đợi tự đóng rồi vào Admin bằng A654321#.
5. Chọn 1, quẹt thẻ thứ ba: báo đầy. Từ menu chọn 2, nhập 03## để xóa một thẻ ảo.
6. Chọn 1, quẹt thẻ thứ ba: lưu vào slot 03, TOTAL:10.
7. Thoát Admin, rời rồi quẹt lại thẻ thứ ba: CARD:03 và mở.
8. Tắt/bật nguồn: thẻ mới vẫn mở, slot 03 vẫn giữ nguyên.
9. Thử PIN sai, đổi PIN, nhập quá thời gian và lỗi OLED theo TEST_PLAN.md.

Tám UID ảo chỉ chiếm slot trong bảng; chúng không phải tám thẻ vật lý đã kiểm thử.
Nếu chưa provision hoặc chưa có UID thật, không thể thực hiện demo vật lý này.
