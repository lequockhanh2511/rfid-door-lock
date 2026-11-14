# Build và nạp — M3

## Khi chưa có phần cứng

Giải nén, mở terminal tại source/ trên Ubuntu. Cần Python3 và GCC host cho test:
```sh
python3 tools/check_contracts.py
python3 tools/run_host_tests.py
```
Host tests chạy C trên máy bằng mô hình SPI/I2C/EEPROM và queue. Firmware thật
không dùng mock. Chưa mô phỏng scheduler FreeRTOS hoặc thực thi CPU ARM trên host.
Bài test tạo file trong build/; không cài package, không truy cập USB hoặc nạp kit.

Build firmware cần **Arm GNU Toolchain** (đã kiểm tra bản 13.3.Rel1, GCC13.3.1).
Nguồn chính thức: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
Dùng executable tương ứng hệ điều hành, đặt arm-none-eabi-gcc trên PATH hoặc:
```sh
export ARM_CC='/duong/dan/toolchain/bin/arm-none-eabi-gcc'
python3 tools/build_firmware.py --target app
python3 tools/build_firmware.py --target m2
```
Thay đường dẫn bằng vị trí compiler bạn đã cài. Script tìm project từ vị trí
của chính nó; tên thư mục giải nén không bị cố định. Không cần CubeMX để build.
App: build/app/rfid_application.{elf,hex,bin,map}; diagnostic: build/m2/.
Mặc định không ghi --target là app (khác bản M2 trước đây).

## Khi đã có NUCLEO-F411RE

Chỉ dùng pin map docs/PIN_MAP.md cho NUCLEO-F411RE/MB1136. Kiểm tra module
3,3V, GND chung, chân EEPROM WP/A0/A1/A2, pull-up I2C và chiều LED trước nạp.
Dùng ST-LINK tích hợp trên Nucleo với cáp USB Mini-B có truyền dữ liệu.
STM32CubeProgrammer dùng để nạp; công cụ debug hỗ trợ ELF/ST-LINK dùng để đọc biến.
Trang hãng: https://www.st.com/en/development-tools/stm32cubeprog.html

Ubuntu trong VirtualBox có thể dùng nếu ST-LINK USB được chuyển vào máy ảo và
công cụ có quyền truy cập USB. Máy chủ Windows và Ubuntu khách không được
chiếm ST-LINK cùng lúc. Cấu hình USB/driver/udev cần xác nhận trên máy thực của bạn;
chưa có lần nạp từ VirtualBox được kiểm chứng trong gói này.

Lần lượt thực hiện:
1. Nạp firmware_diagnostics/rfid_m2_diagnostics.hex, chạy kiểm tra theo BUILD_AND_DIAG.md.
2. Debug ELF diagnostic; quẹt từng thẻ thật rồi **halt CPU** để đọc ổn định
   `diag_last_uid.length` và `diag_last_uid.bytes[0..length-1]`. Resume trước thẻ tiếp theo.
   Chỉ lấy UID khi diag_rfid_poll=APP_OK và diag_card_reads tăng. Đọc theo hex,
   không lấy UID khi không có thẻ hoặc lỗi SPI/RF.
3. Tạo dữ liệu setup bằng đúng hai UID vừa đo. Lệnh dưới đây là mẫu, thay hai chuỗi:
```sh
python3 tools/generate_provision.py --uid1 'UID_HEX_THE_1' --uid2 'UID_HEX_THE_2' --confirm-initial-setup
python3 tools/build_firmware.py --target provision
```
   Ví dụ định dạng UID là AA:BB:CC:DD (chỉ minh họa định dạng, **không dùng làm UID thật**).
   Độ dài cho phép 4, 7 hoặc 10 byte; hai UID phải khác nhau. Tám UID ảo cố định
   FA:CE:00:01..08 được đưa vào slot 03..10; script từ chối trùng.
4. Nạp build/provision/rfid_initial_setup.hex. Xem `provision_status=0` và
   `provision_revision=1` hoặc OLED SETUP SAVED. Nếu cấu hình hợp lệ đã tồn tại,
   trả APP_DENIED (8), không ghi đè. Firmware setup luôn giữ khóa đóng và
   không chạy luồng mở cửa. Không dùng nó làm firmware cuối.
5. Nạp firmware_application/rfid_application.hex (hoặc bản app vừa build).
   EEPROM ngoài vẫn giữ dữ liệu khi nạp lại Flash STM32. Chạy kịch bản USER_GUIDE.md.

HEX đã có địa chỉ; nếu dùng BIN thì Flash base **0x08000000**. Không nạp BIN EEPROM
vào Flash: firmware setup tự gửi dữ liệu sang 24LC256 qua I2C.
Chưa cung cấp công cụ xóa/reset toàn bộ EEPROM để tránh thao tác nhầm. Muốn sửa
thẻ/PIN đã có thì dùng Admin; muốn khôi phục khi quên PIN cần quy trình riêng có chủ đích.

ZIP không chứa firmware setup bằng UID mẫu. `--test-only` chỉ tạo dữ liệu cho
kiểm tra build; firmware dựng ở chế độ đó **không thể ghi EEPROM**. Không dùng
nó để provision. Tạo lại header bằng UID thật trước khi build setup thực.

## Những biến cần đo trên kit

`control_stack_words`, `rfid_stack_words`, `ui_stack_words`: stack còn ít nhất,
đơn vị words; `app_free_heap`: byte heap đang rảnh. Các số 0 trước khi task chạy
không phải kết quả đo. Xem thêm `control_queue_errors`, `control_key_errors`,
`rfid_dropped_events`, `rfid_reader_status`, `ui_display_status`.
Đo thời hạn đóng thực tế, heap/stack, tốc độ quét, mất bus và brownout trước release.
Không có UART log trong bản này.
