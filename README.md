# Khóa cửa RFID — M3 và tích hợp ứng dụng sơ bộ

Đã viết cả ba phần, kiểm thử logic/driver trên host và build ARM cho **NUCLEO-F411RE**.
Ứng dụng có 10 slot thẻ, PIN, Admin, cấu hình EEPROM A/B, OLED/LED và ba task FreeRTOS.
**Chưa chạy trên kit.** Firmware chưa có hai UID thật; EEPROM trống sẽ hiện SETUP REQUIRED.

| Thành viên học | Mã chính |
|---|---|
| Nguyễn Hữu Anh | rc522, eeprom24, config_store, rfid_service, task_rfid |
| Hà Xuân Khải | keypad, access_control, task_control |
| Lê Quốc Khánh | ssd1306, lock_output, task_ui |

Đọc **docs/START_HERE.md** trước. Tài liệu chi tiết: docs/LEARNING.md,
docs/USER_GUIDE.md, docs/BUILD_AND_FLASH.md, docs/STORAGE_FORMAT.md và STATUS.md.

```sh
python3 tools/check_contracts.py
python3 tools/run_host_tests.py
python3 tools/build_firmware.py --target app
```

Hai lệnh đầu cần Python3 + GCC host; lệnh cuối cần Arm GNU Toolchain trên PATH
hoặc biến ARM_CC trỏ đến arm-none-eabi-gcc. Nguồn HAL/CMSIS/FreeRTOS và license
đã nằm trong middlewares/. Không có yêu cầu tải SDK trong lúc build.
Dùng script build, không có .ioc hoặc project CubeIDE GUI giả.

ZIP chứa source/, firmware_application/, firmware_diagnostics/, rfid-door-lock.bundle.
Không chứa firmware provision với UID giả. Công cụ tạo provision nằm trong source/tools/.
Đây là dự án học tập xác thực UID và PIN demo, chưa phải sản phẩm khóa an ninh.
