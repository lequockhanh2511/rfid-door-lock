# Quy tắc dự án RFID

Làm cả ba phần; phân công tên chỉ để học. Không giả danh thành viên trong commit.
Môi trường riêng, không truy cập máy người dùng. Bàn giao ZIP + Git bundle.
main chỉ chứa baseline cho tới khi có release đã kiểm thử kit; tính năng từ develop.
Đọc docs/SPEC.md, docs/DECISIONS.md, STATUS.md trước khi triển khai.
API chung là hợp đồng: đổi thì cập nhật tài liệu và kiểm tra cả ba phần.
Không tự đặt chân board, UID thật, SDK giả, file .ioc giả hoặc log PASS giả.
Không tuyên bố kiểm tra header là firmware build. Không ghi PIN vào log.
Không HAL_Delay trong task; giao dịch ngoại vi có timeout; ISR dùng FromISR.
Không commit build output. Ghi rõ kiểm thử host/firmware/kit riêng biệt.
