/* Le Quoc Khanh: display failures cannot block GPIO lock deadlines. */
#include "app_tasks.h"
#include "app_ipc.h"
#include "device_api.h"
#include "FreeRTOS.h"
#include "task.h"
volatile app_result_t ui_display_status;
volatile uint32_t ui_stack_words,app_free_heap;
static void run(void *arg){
    (void)arg;app_ui_snapshot_t view={0};view.state=APP_STATE_BOOT;
    ui_display_status=oled_init(250);
    for(;;){
        (void)app_ipc_receive_ui(&view,APP_UI_PERIOD_MS);
        if(ui_display_status!=APP_OK)ui_display_status=oled_init(250);
        if(ui_display_status==APP_OK)ui_display_status=oled_present(&view,250);
        ui_stack_words=uxTaskGetStackHighWaterMark(NULL);app_free_heap=(uint32_t)xPortGetFreeHeapSize();
        vTaskDelay(pdMS_TO_TICKS(APP_UI_PERIOD_MS));
    }
}
app_result_t task_ui_create(void){
    return xTaskCreate(run,"UI",APP_UI_STACK_WORDS,NULL,APP_UI_PRIORITY,NULL)==pdPASS?APP_OK:APP_NOT_READY;
}
