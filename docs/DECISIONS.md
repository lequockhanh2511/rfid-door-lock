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

## M3 và tích hợp sơ bộ
- card_store được triển khai bằng rfid_service.c + config_store.c, không tạo lớp rỗng chỉ để đủ tên.
- Thêm occurred_ms trong event và selected_id/confirm_pin/pin_role trong snapshot;
  đã cập nhật cả ba phần và test header/ARM, không có API riêng bị bỏ quên.
- Request timeout3s; tối đa một pending. D hủy quyền/ngữ cảnh, không hoàn tác mutation đã gửi.
- Reply đầy queue được giữ và retry; card event đầy bị bỏ có counter, cần rời thẻ quẹt lại.
- Chỉ ba poll NO_EVENT liên tiếp mới re-arm; lỗi reader không coi là đã rời thẻ.
- Cấu hình A/B tại0x0000/0x0100, bản160 byte, CRC32 và marker. PIN là dữ liệu demo rõ.
- Chưa có UID thật: chỉ giao generator và source provision, không giao image setup bằng UID mẫu.
- Tích hợp main/IPC/build trong integration/system-v1. Host pass không thay nghiệm thu kit.
- Script build mặc định đổi từ diagnostic ở M2 sang app; diagnostic dùng --target m2.
