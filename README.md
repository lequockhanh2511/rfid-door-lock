# Khóa cửa RFID – STM32F103C8T6

Đặc tả chức năng theo HTNN04_N02.pdf và yêu cầu quản lý thẻ mới nhất của nhóm.
App tự tạo cấu hình lần đầu khi cả hai trang Flash cấu hình đều trắng.

## Phương án thẻ duy nhất

- Slot 01 và 02 để trống.
- Slot 03–10 chứa tám UID ảo khác nhau, từ FA:CE:00:01 đến FA:CE:00:08.
- Cấu hình lần đầu có 8/10 thẻ. Thêm hai thẻ vật lý bằng Admin trực tiếp khi demo.
- Thêm thẻ vào số trống nhỏ nhất, từ chối trùng và báo đầy ở 10 thẻ.
- Xóa được mọi slot theo số, giữ nguyên số của những slot khác.
- Khi khởi động lại, tải cấu hình đã lưu. Thẻ ảo đã xóa không được tự khôi phục.
- Hai trang đều 0xFF được coi là chưa khởi tạo. Có dữ liệu nhưng không còn bản
  hợp lệ thì báo FLASH DATA CORRUPT, đóng khóa và giữ dữ liệu để chẩn đoán.
- Nếu còn một bản A/B hợp lệ, tải bản hợp lệ mới nhất. Lỗi đọc/ghi được báo riêng.
- Khởi tạo bị mất nguồn giữa chừng có thể trở thành dữ liệu hỏng. App không tự
  ghi đè trường hợp này. Việc xóa dữ liệu để thiết lập lại cần thao tác bảo trì có chủ ý.

UID ảo được khai báo tại app/inc/initial_config.h. Firmware không có UID dựng
sẵn cho thẻ vật lý. Các UID mô phỏng trong tests chỉ phục vụ kiểm thử.

## Sử dụng

PIN demo mở cửa là 123456, PIN Admin là 654321. Nhập PIN rồi # để xác nhận.
Nhấn A, nhập PIN Admin và #. Menu 1 thêm thẻ, 2 xóa thẻ, 3 đổi PIN.
Khi xóa, nhập số 01–10 và #, rồi # lần nữa để xác nhận.
* hủy/quay lại trước khi gửi lệnh. Lệnh ghi đã gửi sang RFID Task có thể đã
thực hiện khi bấm *. Đổi PIN chọn 1 mở cửa hoặc 2 Admin, nhập PIN hiện tại,
PIN mới và nhập lại PIN mới, mỗi bước kết thúc bằng #.

Sai PIN Admin 3 lần khóa đăng nhập 30 giây. Admin không thao tác 30 giây tự
thoát. Lần xác thực thành công mở khóa trong 5 giây. Khởi động ra lệnh đóng servo.
OLED thể hiện trạng thái lệnh và không có cảm biến xác nhận vị trí cơ khí.

## Kiến trúc và phần cứng

- Nguyễn Hữu Anh phụ trách RFID Task, RC522 và cấu hình Flash.
- Hà Xuân Khải phụ trách Control Task, keypad, Admin và thời hạn điều khiển khóa.
- Lê Quốc Khánh phụ trách UI Task và OLED.
- Ba task trao đổi qua command/event queue và UI mailbox.

RC522 dùng SPI1, keypad 4×4 dùng GPIO, OLED SSD1306 dùng I2C1, SG90 dùng PA0
TIM2_CH1. Bảng chân chi tiết nằm trong docs/NOI_DAY_F103.md. Clock HSI 8 MHz,
xung đóng/mở 1000/1500 µs là giá trị cần hiệu chỉnh trên kit.

## Build và kiểm thử

Repository đã có HAL/CMSIS device F1, startup F103xB, CMSIS core và FreeRTOS
ARM_CM3. Phiên bản nguồn nằm trong middlewares/manifest.json. Chỉ cần GCC host
và ARM GNU Toolchain chạy được. Với bộ portable đã chuẩn bị trong workspace,
chạy từ thư mục repository bằng Python 3.

    python -X utf8 tools/run_host_tests.py --cc ../tmp/toolchains/w64devkit/bin/gcc.exe
    $env:CC = (Resolve-Path ../tmp/toolchains/w64devkit/bin/gcc.exe).Path
    python -X utf8 tools/check_contracts.py
    python -X utf8 tests/test_provision.py
    python -X utf8 tools/build_firmware.py --target app --cc ../tmp/toolchains/bin/arm-none-eabi-gcc.exe

Có thể đặt ARM_CC hoặc CC. Script chỉ điều chỉnh môi trường tiến trình để dùng
compiler portable, kể cả khi workspace có tên tiếng Việt. Tham số --sanitize
của host runner yêu cầu toolchain hỗ trợ UBSan và được ghi rõ trong log.

Target app đã bao gồm khởi tạo 8 thẻ ảo trên Flash trắng, không cần nạp provision
để đăng ký thẻ thật. Target uid là công cụ phụ để xem UID, không ghi cấu hình.
Target provision phụ vẫn chỉ dùng cùng cấu hình 8 thẻ ảo và từ chối ghi đè dữ
liệu đã khởi tạo hoặc bị hỏng. Bản build kiểm thử dùng --test-only.

    python -X utf8 tools/generate_provision.py --test-only
    python -X utf8 tools/build_firmware.py --target provision --cc ../tmp/toolchains/bin/arm-none-eabi-gcc.exe
    python -X utf8 tools/build_firmware.py --target uid --cc ../tmp/toolchains/bin/arm-none-eabi-gcc.exe

ELF/HEX/BIN, map, memory.json và báo cáo stack .su được xuất dưới build/<target>.
Hai trang cấu hình tại 0x0800F800 và 0x0800FC00 nằm ngoài vùng firmware 62 KiB.
Khi nạp lại firmware, giữ hai trang này để bảo toàn thẻ và PIN.

Báo cáo chạy và firmware được xuất riêng, không đưa vào bản mã nguồn nộp.
Chưa nghiệm thu phần cứng thật. Cần đo PWM/góc servo, thời gian tự đóng,
đọc thẻ thật, mất nguồn khi ghi Flash và stack/heap trên kit.
