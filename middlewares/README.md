# Middleware — chưa đưa vào
M1 không chứa FreeRTOS/CMSIS/STM32CubeF4. Không có stub giả kernel.
M2 phải chọn version, nguồn chính thức, license; pin version trong manifest.
Chọn đúng FreeRTOS port Cortex-M4F và cấu hình interrupt/heap cho target.
