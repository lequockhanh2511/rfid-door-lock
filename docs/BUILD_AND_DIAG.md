# Build và kiểm tra M2

## Công cụ
Đã build bằng Arm GNU Toolchain 13.3.Rel1 (GCC 13.3.1), Python3.
Toolchain không đóng gói trong ZIP; nguồn tải chính thức:
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
SDK/HAL/kernel đã nằm trong source/middlewares, kèm license và commit trong manifest.json.
Không yêu cầu CMake, không yêu cầu chạy CubeMX. Không có file .ioc giả.

## Ubuntu — sau khi tự cài toolchain
Ở thư mục source:
```
python3 tools/check_contracts.py
python3 tools/run_host_tests.py
python3 tools/build_firmware.py
```
Nếu compiler chưa có trên PATH, đặt ARM_CC vào đúng executable bằng đường dẫn của bạn.
Script không tự cài phần mềm, không kết nối/nạp board, không thay đổi cấu hình máy.
Kết quả build nằm trong build/m2/; không commit các file build vào Git.
M2 dùng -nostartfiles với startup ST, linker script của dự án, newlib-nano,
FreeRTOS ARM_CM4F + heap_4. SysTick 1 kHz dùng chung tick HAL và RTOS.

## Bản firmware có sẵn
firmware_diagnostics/rfid_m2_diagnostics.hex có địa chỉ Flash sẵn;
ELF phục vụ debug; BIN cần địa chỉ đích 0x08000000 nếu nạp thủ công.
Chỉ dành cho NUCLEO-F411RE theo pin map M2. Không phải firmware khóa cửa hoàn chỉnh.
Chưa nạp lên kit; kiểm tra nguồn/module thực trước dùng.

## Hành vi dự kiến — chưa kiểm chứng trên kit
- LED đỏ sáng, LED xanh tắt; các phím không mở khóa trong M2.
- OLED hiện RFID DOOR LOCK, SETUP REQUIRED; mã RESULT phản ánh đọc EEPROM.
- RC522 đọc UID; giữ thẻ sẽ tăng diag_card_reads nhiều lần (lọc lặp làm ở M3).
- Keypad một nhấn tạo một sự kiện, xem diag_last_key/diag_key_events.
- EEPROM chỉ đọc 16 byte đầu mỗi giây, không tự ghi/xóa/provision.
- Kiểm tra EEPROM write bằng host test đã chạy; test trên kit cần vùng scratch
  hoặc chương trình kiểm tra riêng có chủ đích, không ghi đè dữ liệu người dùng.
- Xem diag_rfid_init, diag_rfid_poll, diag_oled, diag_eeprom trong debugger.
  APP_OK=0, APP_IO_ERROR=5, APP_TIMEOUT=7, APP_NOT_READY=9, APP_NO_EVENT=10.
- diag_*_stack_words là stack còn tối thiểu theo FreeRTOS (words); diag_free_heap
  là heap khả dụng (bytes). Không coi giá trị ban đầu 0 khi chưa chạy là số đo.

Nạp qua ST-LINK bằng STM32CubeProgrammer; debug bằng công cụ hỗ trợ ELF/ST-LINK.
Nếu dùng Ubuntu VirtualBox, cần chuyển ST-LINK USB vào máy ảo và quyền udev phù hợp.
Không cần nạp ngay khi chưa có kit. Không có UART log trong M2.
