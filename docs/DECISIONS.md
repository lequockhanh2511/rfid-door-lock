# Quyết định cập nhật sau đặc tả gốc

Tài liệu này ưu tiên hơn SPEC.md khi mô tả nơi thực hiện và bàn giao.
- Người dùng yêu cầu chỉ làm trong môi trường riêng; không can thiệp máy tính.
- Bàn giao ZIP và Git bundle, không bắt buộc người dùng kết nối GitHub để bắt đầu.
- Codex triển khai cả ba phần. Danh tính commit dùng cấu hình Codex thực tế của môi trường; không dùng tên ba sinh viên làm tác giả giả.
- Nguyễn Hữu Anh = phần 1; Hà Xuân Khải = phần 2; Lê Quốc Khánh = phần 3.
- Không có remote nên chưa có PR/review GitHub. Merge nội bộ không thay cho review ba người.
- M1 có header contract và compile check; không có main.c giả, không giả vờ link firmware thành công.

## M2
- Đã chọn pin map theo docs/PIN_MAP.md; code BSP viết tay theo HAL, không giả .ioc.
- Có bản diagnostic riêng giữ khóa đóng; chưa thay thế app/IPC M3.
- Không tự ghi EEPROM trong diagnostic; host test kiểm tra driver write.
- Loại phụ thuộc snprintf khỏi OLED và dùng startup ST không hosted crt0;
  build cuối không còn cảnh báo. HAL flash_ex nguồn vẫn được giữ nhưng không
  biên dịch vì diagnostic không dùng flash erase/program.
