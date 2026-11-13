# Cách đọc gói M1

1. README.md: trạng thái và cách kiểm tra.
2. SPEC.md: yêu cầu hệ thống; DECISIONS.md: cập nhật môi trường và tên.
3. ARCHITECTURE.md: ai sở hữu dữ liệu và các bước trao đổi.
4. app/inc/app_cfg.h: thông số dự kiến.
5. app/inc/app_types.h và app_ipc.h: dữ liệu giao tiếp.
6. hal/inc/: các hàm mỗi phần sẽ triển khai.

Đây chưa phải project import và nạp được trong CubeIDE. Việc còn lại: driver,
BSP, SDK/kernel, logic, task, main, startup/linker, build ARM, test host và kit.

## Nhánh và lịch sử
Git bundle đi kèm giữ nhánh và commit. Từ thư mục giải nén:
```
git clone -b develop rfid-door-lock.bundle rfid-door-lock-restored
cd rfid-door-lock-restored
git log --all --graph --oneline
```
Không có GitHub remote; origin sau clone là file bundle, không phải server để push.
Trong repository đã clone, main có thể chỉ xuất hiện dưới origin/main;
muốn nhánh local main: `git branch main origin/main`.

## Kiểm tra header (không nạp board)
Trên Ubuntu có Python3 và GCC:
```
python3 tools/check_contracts.py
```
Lệnh chỉ chạy khi người dùng chủ động thực hiện. ZIP không có script tự cài,
script tự nạp hoặc tự sửa máy. Kết quả kiểm tra trong môi trường bàn giao ở
reports/m1-validation.txt. Không cần chạy để đọc tài liệu.
