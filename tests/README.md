# Kiểm tra có thể chạy lại

`python3 tools/run_host_tests.py` dùng GCC C11, Wall/Wextra/Werror/pedantic,
UndefinedBehaviorSanitizer và Python3. Không nạp STM32, không cần peripheral thật.

| Test | Mã được kiểm tra | Mô hình thay thế |
|---|---|---|
| test_storage.c | eeprom24 driver | I2C/EEPROM page và ACK |
| test_rc522.c | rc522 driver | Register/FIFO và phản hồi RF |
| test_keypad.c | scan/debounce | GPIO matrix/tick |
| test_display.c | SSD1306 và lock_output | I2C framebuffer/GPIO |
| test_config.c | config_store + rfid_service | EEPROM byte có fault injection |
| test_control.c | access_control + service + store | Command callback, thời gian, EEPROM |
| test_ipc.c | app_ipc | Queue FIFO model; không thay kernel firmware |
| test_provision.py | generator | UID TEST_ONLY, thư mục tạm riêng |

config:486 byte-cut trials (162 vị trí ×3 ca);1280 single-bit corruption trials;
readback sau commit lỗi; slot/duplicate/full/PIN/reboot/sequence wrap.
Control: timeout5s/20s/60s/3s, PIN/Admin, xác nhận, card stale/cùngtick,
reply muộn, cancel, queuefull, lỗi lưu, blankEEPROM, tickwrap.
OLED: tất cả14 trạng thái×11 result code, bounds, timeout, LED độc lập.
IPC: lỗi tạo mỗiqueue và cleanup; copy/FIFO/full; timeout không vô hạn;
chỉ UI dài1 được overwrite. Chưa mô phỏng lịch task thật/priority inheritance.

`python3 tools/check_contracts.py` chỉ compile header. Firmware:
`python3 tools/build_firmware.py --target app` link SDK/kernel thật bằng ARM GCC.
Không có macro NDEBUG để vô hiệu hóa assert trong host tests.
Không kết luận PASS kit từ các kết quả trên. Xem docs/TEST_PLAN.md cho phần vật lý.
