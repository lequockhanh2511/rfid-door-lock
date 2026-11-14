# Trạng thái M3 — 14/11/2026

## Đã hoàn thành bằng code và kiểm tra trên máy

- Cả ba phần: Nguyễn Hữu Anh (RFID/EEPROM), Hà Xuân Khải (keypad/Admin/control),
  Lê Quốc Khánh (OLED/LED); giữ driver M2 và thêm task/logic M3.
- 10 slot cố định, UID 4/7/10 byte, chống trùng/đầy, xóa không dồn số, dùng lại slot trống thấp nhất.
- EEPROM A/B serialize160 byte, CRC32, commit marker, đọc lại; lỗi lưu buộc reload.
- PIN6 số, hai vai trò, xác nhận PIN mới hai lần, Admin thêm/xóa/list/đổiPIN.
- Hạn mở5s, nhập20s, Admin60s, request3s; xử lý tràn tick, hủy và phản hồi muộn.
- Ba task FreeRTOS thực; IPC copy bằng queue, mailbox UI dài1, GPIO khóa độc lập I2C.
- Công cụ tạo setup từ hai UID do người dùng cung cấp +8 UID ảo. App không tự seed EEPROM.
- Build/link ARM ứng dụng, diagnostic và cả hai nhánh setup (ghi thực/TEST_ONLY) đạt.
- 8 chương trình test host: bốn nhóm driver, config/service, control/service, IPC model,
  generator Python; header check đạt. Trong đó486 thử ngắt ghi,1280 thử lật bit.
- Firmware app linker: **FLASH21040 byte /512KiB, RAM29192 byte /128KiB**.
  Đây là dung lượng liên kết, gồm heap/stack dự trữ; không phải đo stack/heap khi chạy.
- ZIP có source, app/diagnostic ELF/HEX/BIN/MAP, Git bundle, tài liệu và log kiểm tra.

## Còn chờ để nghiệm thu

- **Chưa có kit, chưa nạp hoặc chạy STM32.** Chưa đo stack high-water, heap, jitter,
  RF, mức điện, pull-up, địa chỉ OLED/EEPROM thực hoặc brownout thật.
- Chưa có hai UID thẻ vật lý, chưa provision EEPROM thật, chưa demo đủ10 thẻ.
- Test host thay bus/EEPROM/queue bằng mô hình; chưa chạy scheduler FreeRTOS trên host.
- Chưa có UART, servo/relay hoặc khóa cơ: demo bằng LED đỏ/xanh.
- RC522 chỉ nhận VersionReg0x91/0x92; collision nhiều thẻ bị từ chối.
- Chưa có remote, PR GitHub, review độc lập hoặc release v1.0.0.

## Mốc và Git

M3 logic/task đã hoàn thành kiểm thử trên máy. M4 đã ghép và build sơ bộ;
kiểm thử tích hợp thời gian/nguồn/stack trên kit thuộc phần còn chờ M5.
Các feature/task-* đã merge cục bộ --no-ff vào develop; integration/system-v1
được merge sau kiểm tra gói. Main giữ nguyên baseline M1. Không giả tác giả sinh viên.

Tiếp theo: đọc tài liệu theo phần → khi có kit chạy diagnostic → lấy UID thật
→ provision có chủ đích → nạp app → thực hiện checklist kit và review trước release.
