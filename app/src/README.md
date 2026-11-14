# Luồng ứng dụng

main.c khởi tạo BSP, khóa đóng, mutex, queue, ba task rồi chạy scheduler.
- task_rfid.c gọi driver và rfid_service.c; config_store.c sở hữu dữ liệu A/B.
- task_control.c quét keypad, gọi access_control.c, điều khiển hạn LED khóa.
- task_ui.c chỉ đọc snapshot và cập nhật OLED.
- app_ipc.c bọc FreeRTOS queues; chỉ UI mailbox được overwrite.

access_control/config_store/rfid_service là logic C có test host.
Không gọi test/mock trong firmware. Mọi biến *_stack_words chỉ có giá trị đo
sau khi thực sự chạy trên kit; hiện chưa có số đo đó.
