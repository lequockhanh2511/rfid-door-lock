# Kiểm thử M2
python3 tools/run_host_tests.py chạy bốn chương trình mock test với GCC và UBSan.
EEPROM: chia page/bounds/WP/fault/timeout. RFID: CRC vector/cascade/BCC/SAK/fault.
Keypad: 16 phím/debounce/hold/chord/wrap. OLED: frame/font/clipping/bus/LED.
Không mô phỏng điện, RF hoặc thực thi FreeRTOS trên STM32. Kiểm thử trên kit còn chờ.
