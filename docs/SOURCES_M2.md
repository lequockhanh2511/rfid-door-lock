# Nguồn kỹ thuật đối chiếu M2

- ST UM1724 Rev17, bảng 29 trang 59: NUCLEO-F411RE MB1136 headers; PA13/PA14 SWD.
  https://www.st.com/resource/en/user_manual/DM00105823-.pdf
- ST DS10314 Rev8, bảng alternate function: PA5/6/7 SPI1 AF5; PB8/PB9 I2C1 AF4.
  https://www.st.com/resource/en/datasheet/stm32f411re.pdf
- NXP MFRC522 Rev3.9: SPI register framing, FIFO/IRQ, timer, version register.
  https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
- Microchip DS21203M: EEPROM 32768 byte, page 64 byte, ACK polling, random read.
  https://ww1.microchip.com/downloads/en/DeviceDoc/21203M.pdf
- Solomon Systech SSD1306 Rev1.1 (bản datasheet lưu tại Adafruit): I2C control,
  page mode, charge pump và trình tự khởi tạo.
  https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
- Arm GNU Toolchain 13.3.Rel1 đã tải vào môi trường riêng để build:
  https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
- Nguồn chính thức HAL, CMSIS device/core, FreeRTOS: middlewares/manifest.json.

Pin map và cấu hình bus là lựa chọn thiết kế dựa trên tài liệu; không phải đo trên kit.
