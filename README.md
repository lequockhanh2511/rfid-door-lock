# RFID Door Lock — M1

Khung dự án NUCLEO-F411RE, C, dự kiến STM32CubeF4 HAL và FreeRTOS.
**Chưa phải firmware; không có file nạp .elf/.hex/.bin.**

Ba phần đều do Codex triển khai; tên thành viên là phân công học và trình bày:
- Nguyễn Hữu Anh: RC522, danh sách thẻ, EEPROM.
- Hà Xuân Khải: keypad, PIN, Admin Mode, điều phối.
- Lê Quốc Khánh: OLED, LED khóa, hiển thị.

Bắt đầu đọc docs/START_HERE.md, docs/ARCHITECTURE.md và STATUS.md.
Kiểm tra header trên Linux có GCC và Python: `python3 tools/check_contracts.py`.
Đây là compile check API, không phải kiểm thử chức năng hay phần cứng.

Gói bàn giao có source/ và rfid-door-lock.bundle ở thư mục cha.
Để khôi phục lịch sử: `git clone -b develop rfid-door-lock.bundle rfid-door-lock-restored`.
Không cần khôi phục Git nếu chỉ muốn đọc code.
