# Bắt đầu với gói M3

Bạn chưa cần sửa code hoặc nạp gì khi chưa có phần cứng. Gói này đã chứa code
của cả ba người; phân công để học, không phải ba firmware độc lập.

1. Giải nén ZIP vào thư mục bạn chọn. Mở source/README.md và source/STATUS.md.
2. Học theo source/docs/LEARNING.md và bảng tên file tương ứng từng người.
3. Muốn thử ngay trên Ubuntu: vào source/, chạy `python3 tools/run_host_tests.py`.
   Đây là mô hình kiểm thử C trên máy; không phải mô phỏng điện hay chạy STM32.
4. Đọc USER_GUIDE.md để hiểu thao tác keypad và kịch bản demo.
5. Khi có kit, theo PIN_MAP.md và BUILD_AND_FLASH.md: chẩn đoán → đọc hai UID thật
   → tạo/nạp provision một lần → nạp firmware_application.

| Thư mục/file trong ZIP | Mục đích |
|---|---|
| source/ | Toàn bộ code, SDK cần thiết, test và tài liệu |
| firmware_application/ | ELF/HEX/BIN/MAP ứng dụng M3 đã build; chưa chạy kit |
| firmware_diagnostics/ | Bản M2 chẩn đoán, giữ khóa đóng, chỉ đọc EEPROM |
| rfid-door-lock.bundle | Lịch sử Git và các nhánh thật |
| SHA256SUMS.txt | Checksum các file trong gói |

Nếu muốn học Git, tự chạy ở nơi đã giải nén:
```sh
git clone -b develop rfid-door-lock.bundle rfid-door-lock-restored
cd rfid-door-lock-restored
git log --all --graph --oneline
```
origin lúc này là file bundle, chưa phải GitHub. Nhánh main vẫn ở baseline M1,
chưa có release v1.0.0. Các nhánh còn lại có trong remote-tracking refs sau clone.
Không cần clone nếu chỉ muốn đọc hoặc build từ source/.

Toàn bộ công việc đã làm trong môi trường riêng. Các lệnh trong tài liệu chỉ
chạy trên máy bạn khi bạn chủ động thực hiện; ZIP không tự cài hay tự nạp gì.
