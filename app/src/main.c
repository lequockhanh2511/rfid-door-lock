#include "board.h"
#include "app_ipc.h"
#include "app_tasks.h"
#include "device_api.h"
#include "FreeRTOS.h"
#include "task.h"
int main(void){
    if(board_init()!=APP_OK)board_assert_failed();
    lock_output_init_closed();
    if(board_bus_mutex_init()!=APP_OK||app_ipc_init()!=APP_OK)board_assert_failed();
    if(task_control_create()!=APP_OK||task_rfid_create()!=APP_OK||task_ui_create()!=APP_OK)board_assert_failed();
    vTaskStartScheduler();board_assert_failed();
}
