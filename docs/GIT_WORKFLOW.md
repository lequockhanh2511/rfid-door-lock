# Git thực hiện theo giai đoạn

M1: baseline trên main; tạo develop; nhánh feature/project-contracts thêm
API/docs/check; kiểm tra rồi merge --no-ff về develop.

M2: feature/hal-rfid-storage, feature/hal-keypad, feature/hal-display-lock.
M3: feature/task-rfid, feature/task-keypad-admin, feature/task-display-lock.
M4: integration/system-v1 từ develop.
Chưa tạo các nhánh M2–M4 rỗng để tránh nhầm đã có công việc.

Nếu không có kit, driver/task chỉ ghi kiểm tra host đạt, nghiệm thu kit còn chờ.
Chỉ sau kit đạt mới merge develop về main và tag v1.0.0.
Không có remote/PR ở M1. Sau này cần PR thật thì kết nối repo và review thật.
Không sửa main bằng tính năng sau baseline. Không sửa tác giả commit thành sinh viên.

## Đã thực hiện M2
feature/platform-m2 -> develop (nguồn SDK/BSP); từng nhánh driver -> develop
sau host test; feature/m2-diagnostics -> develop sau ARM build. Chỉ merge cục bộ,
không giả PR/review GitHub. Chưa tạo integration/system-v1 vì chưa có app M3.
