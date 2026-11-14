# Git đã thực hiện

M1: main baseline → develop → feature/project-contracts → merge --no-ff.
M2: feature/platform-m2, feature/hal-rfid-storage, feature/hal-keypad,
feature/hal-display-lock, feature/m2-diagnostics; mỗi nhánh có commit thật và merge.
M3: feature/task-rfid, feature/task-keypad-admin, feature/task-display-lock;
logic tương ứng được host test trước merge vào develop.
M4 sơ bộ: integration/system-v1 từ develop, ghép main/IPC, công cụ provision,
kiểm thử bổ sung, build ARM, cập nhật tài liệu; merge sau kiểm tra trên máy.

Đọc lịch sử thực bằng `git log --all --graph --oneline` sau clone bundle.
Commit dùng danh tính Codex của môi trường, không giả tên ba thành viên.
Không có remote GitHub, không có PR hay review độc lập. Merge cục bộ và test
là bằng chứng đã làm, không được đổi tên thành PR review đã hoàn tất.

Chưa có kit nên trạng thái nghiệm thu **PENDING_HARDWARE**. Main giữ baseline;
chưa merge develop về main, chưa tag v1.0.0. Khi có kit và checklist đạt, ba
thành viên review phần của mình rồi mới làm release và báo cáo kết quả vật lý.
Tài liệu nguồn quy trình: QuyTrinh-Git-2.pdf giữ trong docs/.
