# Trạng thái M2

## Đã thực hiện
- Pin map NUCLEO-F411RE/MB1136 đối chiếu UM1724 và datasheet; chưa xác nhận kit thật.
- RC522 UID 4/7/10 byte: WUPA, anticollision/select từng tầng, BCC/CRC, HALT, timeout.
- EEPROM: đọc, ghi chia page 64 byte, ACK polling, readback, giới hạn bộ nhớ.
- Keypad: quét row open-drain, chống dội, giữ phím không lặp, chặn tổ hợp phím.
- OLED: framebuffer 128x64, font tự viết, text, page transfer; LED độc lập OLED.
- STM32 HAL, CMSIS và FreeRTOS có nguồn/license/hash; BSP HSI 16MHz, I2C mutex.
- Ba task chẩn đoán chạy driver, trạng thái xem qua biến diag_* trong debugger.
- Host mock tests và header checks đạt; firmware ARM build/link đạt không cảnh báo ở bản cuối.
- Linker: FLASH 14732 B / 512 KiB; RAM 29192 B / 128 KiB. RAM gồm vùng dự trữ;
  không phải số đo high-water mark khi chạy. Các số chỉ áp dụng firmware M2.

## Chưa hoàn thành
- Chưa chạy kit, chưa xác nhận mức điện, RF, pull-up, địa chỉ OLED/module EEPROM.
- Chưa có UID thật, chưa có bộ demo đủ 10 UID và chưa provision.
- Chưa có card_store, cấu hình A/B chống mất điện, PIN/Admin, máy trạng thái và app_ipc.
- Driver EEPROM có thể ghi dở nhiều page; bản ghi nguyên tử sẽ làm ở M3.
- RC522 từ chối collision nhiều thẻ; chỉ nhận VersionReg 0x91/0x92, bản clone khác báo NOT_READY.
- rc522_poll hiện trả UID mỗi lần đọc; chống quẹt lặp/đợi rời thẻ thuộc task M3.
- UI hiện là giao diện driver/diagnostic, chưa phải menu quản trị hoàn chỉnh.
- Chưa có PR GitHub hoặc review độc lập; đã tạo commit và merge cục bộ thực.
- M2 hoàn tất phần code và kiểm tra trên máy; nghiệm thu phần cứng PENDING_HARDWARE.

Tiếp theo M3: logic thuần C + test trạng thái, PIN, card_store, record A/B,
queue/IPC, ba task ứng dụng. Main vẫn ở baseline, chưa có release v1.0.0.
