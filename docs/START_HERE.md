# Đọc gói M2

1. STATUS.md: điều đã kiểm chứng và điều còn thiếu.
2. PIN_MAP.md: cấu hình nối dây đã chọn cho board MB1136.
3. LEARNING.md: đường đọc code của từng người.
4. BUILD_AND_DIAG.md: build/chạy chẩn đoán sau khi có kit.
5. reports/: kết quả kiểm tra thực; không phải kết quả chạy phần cứng.

ZIP gồm source/ (develop), firmware_diagnostics/ (ELF/HEX/BIN/MAP) và Git bundle.
Khôi phục lịch sử, chỉ khi bạn chủ động chạy:
```
git clone -b develop rfid-door-lock.bundle rfid-door-lock-restored
cd rfid-door-lock-restored
git log --all --graph --oneline
```
origin ở đây là file bundle, không phải GitHub để push. main vẫn là baseline M1;
các nhánh remote-tracking phục hồi từ bundle không tự thành nhánh local.
Source ZIP đủ để đọc/build; không cần clone nếu chưa học Git.
Đặc tả SPEC.md giữ nguyên bản đầu để truy vết; DECISIONS.md/STATUS.md cập nhật thực tế.
