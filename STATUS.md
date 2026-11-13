# Trạng thái M1

- Đã có khung, API dùng chung, thông số khởi điểm, phân công ba người.
- Đã có script kiểm tra header C11; kết quả thực trong reports/m1-validation.txt.
- Git: baseline main, feature/project-contracts, merge develop; không remote/PR.
- Môi trường có GCC 14.2.0 và Git 2.52.0; không thấy arm-none-eabi-gcc hoặc cmake trên PATH.
- Chưa đưa SDK STM32CubeF4, CMSIS, FreeRTOS vào repo. Chưa có .ioc, startup/linker hoặc main.c.
- Chưa build firmware, chưa có UID thật, pin map hoặc kit test.
- Bước kế tiếp M2: kiểm chứng tài liệu hãng và pin map, lấy SDK/kernel chính thức,
  chuẩn bị build ARM, triển khai ba nhánh driver với mock test phù hợp.
- Không có release v1.0.0. Gói M1 là checkpoint để tiếp tục, không phải sản phẩm hoàn thiện.
