# BSP M2
NUCLEO-F411RE MB1136, HSI 16 MHz, SPI1 1 MHz, I2C1 100 kHz.
Pin map ở docs/PIN_MAP.md. board.c cấu hình có chủ đích, không phải CubeMX-generated.
ports_stm32.c dùng STM32 HAL và mutex FreeRTOS. runtime.c hỗ trợ startup ST.
Chưa xác nhận pin/clock bằng oscilloscope/logic analyzer hoặc nạp kit.
