#include "access_control.h"
#include "app_tasks.h"
#include "device_api.h"
#include "hal_ports.h"
#include "keypad.h"
#include "FreeRTOS.h"
#include "task.h"
volatile uint32_t control_stack_words,control_key_errors,control_queue_errors;
static app_result_t send_command(const app_command_t *cmd,void *ctx){
    (void)ctx;app_result_t r=app_ipc_send_command(cmd,0);if(r!=APP_OK)control_queue_errors++;return r;
}
static void run(void *arg){
    (void)arg;access_control_t c;keypad_state_t keypad;
    uint32_t now=port_time_ms();keypad_state_init(&keypad,now);access_control_init(&c,now,send_command,NULL);
    TickType_t last=xTaskGetTickCount();uint32_t last_ui=now-APP_UI_PERIOD_MS;
    for(;;){
        now=port_time_ms();access_control_tick(&c,now);
        lock_output_set(c.view.unlocked); /* Deadline first, independent of display/bus. */
        uint16_t mask;char key;
        if(keypad_read_matrix(&mask)==APP_OK){if(keypad_update(&keypad,mask,now,&key)==APP_OK)access_control_key(&c,key,now);}
        else control_key_errors++;
        for(unsigned i=0;i<4;i++){app_event_t e;if(app_ipc_receive_event(&e,0)!=APP_OK)break;access_control_event(&c,&e,port_time_ms());}
        access_control_tick(&c,port_time_ms());lock_output_set(c.view.unlocked);
        if((uint32_t)(now-last_ui)>=APP_UI_PERIOD_MS){(void)app_ipc_publish_ui(&c.view);last_ui=now;}
        control_stack_words=uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&last,pdMS_TO_TICKS(APP_KEYPAD_SCAN_MS));
    }
}
app_result_t task_control_create(void){
    return xTaskCreate(run,"CONTROL",APP_CONTROL_STACK_WORDS,NULL,APP_CONTROL_PRIORITY,NULL)==pdPASS?APP_OK:APP_NOT_READY;
}
