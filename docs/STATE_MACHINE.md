# Bảng trạng thái control

Mọi vòng control gọi tick trước input/event. Mở khóa là cờ độc lập với màn hình,
để WAIT_RESULT/PIN_ENTRY không làm mất deadline khi đã mở.

| Trạng thái | Nhập/sự kiện chính | Kết quả |
|---|---|---|
| BOOT | init gửi GET_STATUS | WAIT_RESULT; không đọc được → ERROR |
| SETUP_REQUIRED | D | Đọc lại; không dùng PIN demo tự động |
| IDLE | Số, A, thẻ | PIN_ENTRY, ADMIN_AUTH hoặc LOOKUP_CARD |
| PIN_ENTRY | Số/*/#/D, thẻ | Verify PIN hoặc lookup; quá20s hủy |
| UNLOCKED | Thẻ hoặc PIN mới | Gia hạn nếu hợp lệ; sau5s đóng; A bị bỏ |
| ADMIN_AUTH | PIN và # | OK mới cấp phiên, sai về IDLE |
| ADMIN_MENU | 1/2/3/4/5/D | Thêm/xóa/list/đổi user/đổi admin/thoát |
| ADD_WAIT_CARD | Thẻ mới | ADD_CARD; thẻ từ ngữ cảnh cũ bị bỏ |
| DELETE_INPUT | Hai số và # | 01..10 → DELETE_CONFIRM; quá20s hủy |
| DELETE_CONFIRM | # hoặc D | DELETE_CARD hoặc về menu |
| LIST_CARDS | 4/6/D | Chọn slot trước/sau, hoặc về menu |
| CHANGE_PIN | Sáu số hai lần | SET_PIN khi khớp; lệch nhập lại |
| WAIT_RESULT | Kết quả trùng ID+kind | Áp kết quả còn hiệu lực;3s lỗi; D đánh dấu hủy |
| ERROR | D | GET_STATUS reload; luôn đóng |

Bất kỳ trạng thái Admin nào không thao tác60s: xóa quyền. Nếu đang chờ mutation,
không hủy vật lý giao dịch, chỉ hủy ngữ cảnh; sau phản hồi quay IDLE.
Kết quả PIN/lookup đến sau hủy/timeout không được mở khóa.
IO_ERROR/NOT_READY/TIMEOUT từ service: xóa quyền, đóng, yêu cầu phục hồi.
Các thao tác quản trị được service xử lý tuần tự; service không nhận input trực tiếp.
