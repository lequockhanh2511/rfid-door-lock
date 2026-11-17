# Thư viện nhúng

Các nguồn được lưu cục bộ để build không cần tải thư viện. Manifest ghi URL và
commit của từng thành phần. License gốc được giữ trong từng thư mục.

- CMSIS core 5.9.0, commit 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c.
- FreeRTOS Kernel 11.1.0, commit dbf70559b27d39c1fdb68dfb9a32140b6a6777a0,
  bổ sung đúng port GCC/ARM_CM3 từ cùng commit chính thức.
- STM32F1 HAL 1.1.10, commit baeff0a8dcb23c72012170a0978254a238f1f980.
- CMSIS device F1, commit c8e9a4a4f16b6d2cb2a2083cbe5161025280fb22,
  gồm header, system_stm32f1xx.c và startup_stm32f103xb.s.

HAL/device F1 được tái sử dụng từ bản clone ST sẵn có trong FreeRTOS_Week02/Bai_02
trên máy. Đã kiểm tra remote, commit và working tree sạch trước khi sao chép.
BSP chọn STM32F103xB; linker giới hạn đúng Flash 64 KiB và RAM 20 KiB của C8.
Các nguồn HAL/device F4 và port CM4F đã loại khỏi nhánh này.
