# RFID Door Lock — M2

Đã triển khai driver ba phần và build firmware **chẩn đoán** cho NUCLEO-F411RE.
Chưa có mật khẩu/Admin/quản lý 10 thẻ/logic tự đóng hoàn chỉnh (M3).
Chưa kiểm tra trên kit. Bản M2 giữ LED khóa đóng, không cấp quyền mở cửa.

- Anh: hal/src/rc522.c, eeprom24.c.
- Khải: hal/src/keypad.c.
- Khánh: hal/src/ssd1306.c, lock_output.c.
- Nền tảng: bsp/, middlewares/, examples/m2_diagnostics/main.c.

Đọc docs/START_HERE.md, docs/PIN_MAP.md, docs/LEARNING.md, STATUS.md.
Host tests: `python3 tools/run_host_tests.py` (Python3 + GCC).
Build ARM: `python3 tools/build_firmware.py` (Arm GNU Toolchain trên PATH hoặc ARM_CC).
Không cần tải SDK khi build vì mã nguồn cần dùng đã nằm trong middlewares/ cùng license.
Không có .ioc hoặc project CubeIDE GUI đã tạo; M2 dùng script build có thể chạy Ubuntu.
Gói ZIP có source/, firmware_diagnostics/ và rfid-door-lock.bundle.
