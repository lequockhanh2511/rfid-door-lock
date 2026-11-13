# Hệ thống khóa cửa RFID — đặc tả và bàn giao Codex v1

Ngày: 12/11/2026. Trạng thái: thiết kế trước triển khai; chưa có phần cứng, chưa có firmware đã build hoặc kiểm thử trên kit, chưa có repository remote được cung cấp.

## 1. Cấu hình đã thống nhất

- Một NUCLEO-F411RE, ST-LINK tích hợp; cáp USB Mini-B truyền dữ liệu.
- RC522 SPI, ba thẻ/tag MIFARE Classic 1K/S50 13,56 MHz.
- Keypad ma trận 4×4, OLED SSD1306 128×64 I2C 4 chân.
- EEPROM ngoài 24LC256 qua I2C, LED đỏ/xanh với điện trở 330 ohm riêng cho mỗi LED.
- Breadboard, dây nối; giai đoạn đầu dùng nguồn USB, chưa dùng servo/relay.
- STM32CubeIDE, STM32CubeMX, STM32CubeF4 HAL, FreeRTOS, C, Git/GitHub.
- SPI1 cho RC522; I2C1 dùng chung OLED và EEPROM; GPIO cho keypad/LED; UART log qua ST-LINK nếu cấu hình board hỗ trợ.
- Chưa chốt chân GPIO cụ thể. Trước khi sinh cấu hình phải đối chiếu sơ đồ đúng revision board, datasheet và các chân ST-LINK/SWD, LED onboard, UART. Không dùng bảng này làm sơ đồ nối dây.
- Tất cả tín hiệu module dự kiến ở 3,3 V. Kiểm tra pull-up I2C về 3,3 V, địa chỉ và ngân sách dòng theo module thực tế.

## 2. Phạm vi và mặc định triển khai

Các giá trị thời gian/mật khẩu dưới đây là mặc định thiết kế, tập trung trong app_cfg.h để sửa dễ dàng.

| Nội dung | Quy tắc v1 |
|---|---|
| Mở bằng RFID | UID hợp lệ mở khóa, hiện số thẻ; UID lạ không mở |
| Mở bằng PIN | PIN người dùng 6 chữ số; nhập đủ rồi nhấn # |
| Mật khẩu demo | PIN người dùng 123456; PIN quản trị 654321, chỉ dùng cho demo |
| Thời gian mở | 5 giây; yêu cầu hợp lệ mới khi đang mở bắt đầu lại thời hạn 5 giây |
| Thời gian nhập | Không thao tác 20 giây thì hủy phần nhập hiện tại |
| Admin | Vào bằng A từ màn hình chờ, nhập PIN admin rồi # |
| Timeout Admin | Không thao tác 60 giây thì thoát và xóa trạng thái xác thực |
| Danh sách thẻ | Tối đa 10 vị trí cố định 01–10; không dồn số khi xóa |
| Thêm | Chọn vị trí trống có số nhỏ nhất; không thêm UID trùng |
| Xóa | Nhập đúng hai chữ số 01–10, xác nhận trước khi ghi |
| Lưu | Chỉ báo thành công sau khi dữ liệu được ghi và kiểm tra lại |
| Khởi động | Đầu ra khóa về trạng thái đóng trước khi khởi động task |
| OLED | Chuỗi không dấu trong v1 để dùng font gọn; tài liệu giải thích bằng tiếng Việt |

Đây là mô hình học tập xác thực theo UID, không phải thiết kế khóa chống sao chép cho sử dụng thực tế.

## 3. Quy ước keypad

| Phím | Tác dụng |
|---|---|
| 0–9 | Nhập số |
| # | Xác nhận |
| * | Xóa ký tự cuối |
| D | Hủy thao tác / trở về; ở menu Admin thì thoát Admin |
| A | Từ IDLE: yêu cầu vào Admin |
| B, C | Chưa sử dụng trong v1 |

Menu Admin: 1 thêm thẻ; 2 xóa thẻ; 3 xem vị trí thẻ; 4 đổi PIN người dùng; 5 đổi PIN admin. Đổi PIN yêu cầu nhập mới hai lần. Xem danh sách dùng 4/6 để lùi/tiến, D để về menu. Không cung cấp reset dữ liệu dễ bấm nhầm trong menu v1; khởi tạo demo là công cụ riêng.

Khi ở Admin, thẻ quẹt không mở khóa. Chỉ chế độ ADD_WAIT_CARD mới sử dụng UID để thêm. Chỉ cho vào Admin khi khóa đang đóng. Khi đang nhập PIN mở cửa, thẻ hợp lệ được phép mở và hủy bộ đệm PIN. Giữ thẻ trước đầu đọc chỉ tạo một sự kiện; cần rời thẻ rồi quẹt lại. Trường hợp nhiều phím cùng nhấn: bỏ qua đến khi nhả hết để tránh nhập sai.

## 4. Dữ liệu demo và lưu trữ

- Chưa có UID thật. Không tự điền UID ngẫu nhiên rồi gọi đó là thẻ vật lý đã đăng ký.
- Host test dùng UID tổng hợp được đánh dấu TEST_ONLY. Bộ demo phần cứng gồm hai UID đọc từ thẻ thật và tám UID giả lập khác nhau.
- Trước khi biết hai UID thật, firmware có thể dùng danh sách trống để đăng ký hoặc công cụ provision riêng; chưa đạt tiêu chí demo đủ 10 thẻ.
- Sau provision, khởi động bình thường đọc EEPROM, không nạp đè danh sách mẫu. Xóa thẻ phải còn hiệu lực sau tắt/bật nguồn.
- So khớp cả độ dài UID và byte UID; kiểu dữ liệu dành tối đa 10 byte. Driver phải xác định rõ những loại UID đã hỗ trợ và kiểm thử.
- Mỗi slot lưu used, uid_len, uid[]; số hiệu là index + 1. Tám UID giả lập chiếm tám vị trí thật trong bảng nhưng không chứng minh đã quẹt tám thẻ vật lý.
- Đề xuất lưu hai bản ghi A/B: magic, version, sequence, payload, checksum và dấu commit. Ghi bản không hoạt động, kiểm tra lại, ghi dấu commit cuối; khởi động chọn bản hợp lệ mới nhất. Dữ liệu phải serialize rõ ràng, không ghi trực tiếp struct có padding.
- Tôn trọng ranh giới page và chu kỳ ghi của EEPROM theo datasheet. Chỉ khóa mutex bus trong giao dịch cần thiết, không giữ mutex suốt khoảng chờ ghi nội bộ.
- Nếu cả hai bản không hợp lệ: báo CONFIG ERROR/SETUP REQUIRED và không mở bằng thông tin chưa xác nhận. Provision lần đầu là quy trình có chủ đích; không tự khôi phục mật khẩu demo khi dữ liệu hỏng.

## 5. Phân công và quyền sửa file

| Người | Sở hữu chính | Không tự sửa |
|---|---|---|
| 1 | rc522, card_store, config_store/EEPROM, task_rfid | main.c, cấu hình CubeMX, giao diện keypad |
| 2 | keypad, pin_input, access_control, admin, task_control | driver RFID/OLED, cấu hình chung |
| 3 | ssd1306, ui_render, lock_output, task_ui | dữ liệu thẻ, xác thực PIN |
| Đầu mối tích hợp | BSP, cấu hình clock/chân, FreeRTOSConfig.h, app_ipc, app_cfg, main.c | Không đổi API đơn phương sau khi đã chia nhánh |

Ba phần tương ứng ba khối chức năng. Chưa có đề gốc xác nhận yêu cầu “mỗi người ba ngoại vi/module”; không tuyên bố phân công này đáp ứng một tiêu chí chưa được kiểm tra.

## 6. Task, IPC và quyền sở hữu trạng thái

- task_control: quét keypad theo chu kỳ, chống dội, sở hữu máy trạng thái và hạn tự đóng. Không thực hiện I2C hoặc EEPROM chặn trong đường điều khiển.
- task_rfid: quét RC522, sở hữu cơ sở dữ liệu thẻ và cấu hình bền vững; xử lý lệnh tra cứu/thêm/xóa/đổi PIN. Không log PIN rõ.
- task_ui: nhận snapshot hiển thị, cập nhật OLED theo nhịp giới hạn. Đầu ra LED do control gọi API lock_output ngắn, không chặn, do người 3 triển khai. Như vậy OLED bị lỗi không làm trễ đóng khóa.
- Một mutex I2C dùng chung OLED/EEPROM. Một task duy nhất sở hữu SPI RC522.
- q_control_events nhận sự kiện từ RFID; q_rfid_commands nhận yêu cầu từ control; q_ui_snapshot dài 1 chứa trạng thái hiển thị mới nhất (có thể overwrite).
- Thông điệp nghiệp vụ có request_id, loại lệnh, dữ liệu sao chép theo giá trị, mã kết quả. Không đưa con trỏ tới biến stack vào queue. Mỗi lần chỉ có một yêu cầu quản trị đang chờ; bỏ kết quả cũ khi người dùng hủy/timeout.
- Lệnh sửa cấu hình đã bắt đầu ghi không bị hoàn tác giả khi UI hủy. Kết quả thực tế vẫn cập nhật dữ liệu; UI phải hiển thị trạng thái đã lưu khi phù hợp. Dùng request_id để tránh áp kết quả vào thao tác mới.
- Kết quả: OK, DUPLICATE, FULL, NOT_FOUND, INVALID_INPUT, IO_ERROR, BUSY, TIMEOUT.
- Queue đầy: trả BUSY/ghi bộ đếm lỗi; không âm thầm báo thành công. UI snapshot được phép bỏ trạng thái hiển thị cũ, lệnh nghiệp vụ không được overwrite.
- Task control ưu tiên cao hơn RFID/UI nhưng luôn nhường CPU. Stack/heap, độ ưu tiên và queue depth là cấu hình khởi điểm cần đo trên kit; không gọi số dự kiến là đã tối ưu.
- Trong task dùng delay FreeRTOS; ISR chỉ dùng API FromISR phù hợp, cấu hình ưu tiên ngắt đúng. Không giữ mutex rồi chờ phản hồi từ task khác.

Máy trạng thái tối thiểu: BOOT, SETUP_REQUIRED, IDLE, PIN_ENTRY, UNLOCKED, ADMIN_AUTH, ADMIN_MENU, ADD_WAIT_CARD, DELETE_INPUT, DELETE_CONFIRM, LIST_CARDS, CHANGE_PIN, WAIT_RESULT, ERROR. Mỗi trạng thái cần bảng sự kiện/hành động/thời hạn trong tài liệu triển khai.

## 7. Quy trình repository

Áp dụng QuyTrinh-Git-2.pdf người dùng cung cấp: khởi tạo khung trên main rồi tạo develop; sau đó không commit tính năng trực tiếp main.

1. Baseline: BSP, API chung, IPC contract, tài liệu build, .gitignore. Một người duy nhất sinh cấu hình CubeMX.
2. Driver: feature/hal-rfid-storage, feature/hal-keypad, feature/hal-display-lock; review và PR về develop.
3. Task: feature/task-rfid, feature/task-keypad-admin, feature/task-display-lock; tạo từ develop đã có HAL. Driver tests chưa chạy kit phải ghi PENDING_HARDWARE. Nếu tiếp tục task với mock trước khi có kit, ghi đây là phát triển sơ bộ, chưa hoàn tất mốc nghiệm thu HAL của quy trình.
4. Ghép: integration/system-v1; đầu mối tích hợp sửa main.c và app_ipc.c, kiểm tra stack/heap, lỗi đồng thời.
5. Sau kiểm thử kit: integration về develop, develop về main, tag v1.0.0.

Không tạo commit giả hoặc tác giả giả. Không có remote thì chỉ chuẩn bị cục bộ; không gọi merge cục bộ là PR GitHub. Khi cần danh tính commit, dùng cấu hình người dùng đã có hoặc hỏi; không tự đặt tên/email người dùng. Không dùng force-push lên nhánh chung. Không commit build/, Debug/, Release/, file sinh tạm.

## 8. Mốc thực hiện và bằng chứng

| Mốc | Đầu ra | Tiêu chí hoàn thành |
|---|---|---|
| M0 | Đặc tả hiện tại | Cấu hình và phân công đã thống nhất; chân/UID còn chờ |
| M1 | Repository khung, API, hướng dẫn build | Cấu trúc rõ; kiểm tra công cụ thực tế; không bịa kết quả firmware build |
| M2 | Driver ba phần | Build và kiểm tra độc lập; kiểm tra kit ghi trạng thái riêng |
| M3 | Logic/task | Test PIN, trạng thái, dữ liệu, lỗi queue/storage; test có tiêu chí cụ thể |
| M4 | Tích hợp | Firmware build bằng toolchain ARM; host test đạt; kit còn chờ nếu chưa có |
| M5 | Kiểm thử kit | Các tình huống bên dưới đạt với log/quan sát thực tế |
| M6 | Release, báo cáo | Chỉ ghi số đo và kết quả đã có |

Kiểm thử bắt buộc: thẻ đúng/sai/trùng; đầy 10 thẻ; xóa 02 giữ số các thẻ khác; thêm lại dùng 02; PIN đúng/sai/hủy/timeout; Admin sai không được sửa; thẻ trong Admin không mở cửa; tự đóng 5 giây kể cả khi OLED lỗi; quẹt liên tiếp; giữ thẻ; đồng thời keypad/RFID; EEPROM lỗi không báo đã lưu; khởi động lại giữ dữ liệu; ghi dở vẫn phục hồi bản hợp lệ; queue đầy; tick wraparound; stack/heap trên kit.

Demo cuối: bật nguồn -> thấy đủ 10 slot -> quẹt thẻ hợp lệ -> tự đóng -> thẻ lạ bị từ chối -> PIN đúng -> vào Admin -> thêm khi đầy -> xóa một UID giả lập -> thêm thẻ lạ -> quẹt thẻ mới được phép -> tắt/bật nguồn -> dữ liệu còn nguyên.

## 9. Prompt giao Codex: giai đoạn đầu

Sao chép nội dung dưới đây vào Codex và đính kèm tài liệu này cùng QuyTrinh-Git-2.pdf:

> Hãy triển khai giai đoạn M1 cho dự án RFID theo đặc tả đính kèm. Trước tiên đọc AGENTS.md và kiểm tra repository hiện tại, không ghi đè công việc có sẵn. Người dùng chưa có phần cứng. Dùng NUCLEO-F411RE, STM32CubeF4 HAL, FreeRTOS và C. Tạo cấu trúc bsp/, hal/inc/, hal/src/, middlewares/, app/inc/, app/src/, tests/, docs/. Viết API contract, kiểu thông điệp và app_cfg.h theo đặc tả; tài liệu phân công, Git workflow, test plan và STATUS.md. Tách logic thuần C để kiểm thử trên host ở giai đoạn sau. Kiểm tra toolchain/SDK thực có, báo thiếu rõ ràng. Nếu chưa có CubeMX hoặc SDK, không tự tạo file .ioc hoặc middleware giả để gọi là firmware hoàn chỉnh. Chưa gán GPIO cho đến khi đối chiếu tài liệu ST đúng board; ghi bảng chân dự thảo riêng. Không tự điền UID thẻ thật. Không triển khai tất cả driver/task trong lượt M1. Nếu có repo Git, tuân thủ nhánh và danh tính commit hiện có; nếu chưa có, chuẩn bị khung và báo thông tin còn thiếu để tạo Git đúng quy trình. Không push hoặc tạo PR khi chưa có remote/quyền truy cập. Kết thúc báo danh sách file, kiểm tra đã chạy, giới hạn, nhánh/commit thực có và bước M2; cập nhật STATUS.md. Giải thích ngắn bằng tiếng Việt để người học hiểu.

## 10. Những việc người dùng cần làm tiếp

1. Tạo hoặc cung cấp repository GitHub và môi trường Codex sử dụng; đây là đầu vào để thao tác remote thực sự.
2. Mua đúng bộ linh kiện đã chốt; gửi ảnh module để xác nhận chân/nguồn trước khi nối.
3. Cài môi trường phát triển theo hướng dẫn sẽ cung cấp cho máy Windows.
4. Khi có kit, đọc UID hai thẻ thật và provision dữ liệu demo.

Không cần tự viết code. Người dùng vẫn thực hiện thao tác nối dây/nạp/thử vật lý và học theo tài liệu từng phần.

## 11. Tài liệu hãng để đối chiếu khi triển khai

- NUCLEO-F411RE: https://www.st.com/en/evaluation-tools/nucleo-f411re.html
- MFRC522: https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf
- 24LC256: https://www.microchip.com/en-us/product/24lc256
- Quy trình Git: QuyTrinh-Git-2.pdf đính kèm trong cuộc trao đổi.

Các tài liệu trên là đầu vào kiểm chứng triển khai. Tài liệu hiện tại chưa chứng nhận pin mapping, firmware build hoặc kết quả phần cứng.
