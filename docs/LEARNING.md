# Học theo ba phần

## Nguyễn Hữu Anh — RFID và EEPROM
Đọc hal/inc/eeprom24.h -> hal/src/eeprom24.c -> tests/test_storage.c.
Thử giải thích: vì sao ghi từ địa chỉ 63 với 130 byte phải chia bốn giao dịch;
ACK polling khác chờ cố định thế nào; readback phát hiện WP; vì sao ghi nhiều
page chưa đảm bảo nguyên tử khi mất điện.
Tiếp theo rc522_protocol.h -> rc522.c -> test_rc522.c: mã thanh ghi SPI, FIFO,
WUPA, BCC, cascade UID 4/7/10, CRC_A của SAK, HALT. Driver không tự biết thẻ nào
được mở cửa; quản lý danh sách sẽ làm M3. Hai thẻ cùng trường RF có thể bị từ chối.

## Hà Xuân Khải — keypad và phần điều khiển sắp triển khai
Đọc keypad.h -> keypad.c -> test_keypad.c. Row 0 là hàng 1, bit 0 là phím 1;
chuỗi mapping 123A456B789C*0#D. Giải thích row open-drain, columns pull-up,
30 ms ổn định, latched chống lặp, blocked khi nhiều phím, phép trừ unsigned
khi bộ đếm thời gian quay vòng. Phần PIN/Admin/task điều khiển chưa viết trong M2.

## Lê Quốc Khánh — OLED và LED
Đọc ssd1306.h -> ssd1306.c -> font5x7.inc -> lock_output.c -> test_display.c.
Framebuffer 1024 byte = 128 cột × 8 page; một byte chứa 8 pixel dọc.
Font 5x7 dùng 5 byte/cột ký tự và một cột trống; 21 ký tự một hàng.
Byte I2C 0x00 là command, 0x40 là data. Mỗi lần gửi 16 byte pixel để giảm
thời gian chiếm mutex; gửi lại toàn frame nếu lần trước bị lỗi.
LED được đổi qua GPIO, không đi qua queue hiển thị hoặc chờ I2C. Ở M3 Khải
quyết định mở/đóng, gọi hàm LED do phần bạn cung cấp. oled_present chỉ nhận số
ký tự PIN để vẽ dấu *, không nhận PIN rõ.

## Cả ba cùng hiểu
bsp/src/ports_stm32.c nối driver với HAL và mutex. Host test thay file này bằng
mô hình giả lập, nên PASS không chứng minh dây nối đúng. main chẩn đoán chỉ là
bài kiểm tra ghép driver, không phải main sản phẩm. Không học mã middleware
trước; ưu tiên code tự viết ở hal/, app/, bsp/.
