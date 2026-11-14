#include "app_tasks.h"
#include "rfid_service.h"
#include "device_api.h"
#include "hal_ports.h"
#include "FreeRTOS.h"
#include "task.h"
volatile uint32_t rfid_dropped_events,rfid_stack_words;
volatile app_result_t rfid_reader_status;
static void run(void *arg) {
    (void)arg;config_store_t store={0};card_gate_t gate={0};
    app_event_t reply={0};bool reply_pending=false;
    rfid_reader_status=rc522_init();uint32_t last_retry=port_time_ms();
    for(;;){
        /* Keep undelivered replies and stop consuming commands until accepted. */
        if(reply_pending&&app_ipc_send_event(&reply,0)==APP_OK)reply_pending=false;
        if(!reply_pending){
            app_command_t cmd;
            if(app_ipc_receive_command(&cmd,0)==APP_OK){
                rfid_service_command(&store,&cmd,&reply);app_secure_clear(&cmd,sizeof cmd);
                reply_pending=app_ipc_send_event(&reply,0)!=APP_OK;
            }
        }
        if(rfid_reader_status==APP_OK){
            app_uid_t uid;bool present=false;app_result_t r=rc522_poll(&uid,&present,150);
            if(card_gate_update(&gate,r,present)){
                app_event_t e={0};e.kind=APP_EVENT_CARD_PRESENT;e.uid=uid;e.occurred_ms=port_time_ms();
                if(app_ipc_send_event(&e,0)!=APP_OK)rfid_dropped_events++;
            }
            if(r!=APP_OK&&r!=APP_NO_EVENT){rfid_reader_status=r;last_retry=port_time_ms();}
        }else if((uint32_t)(port_time_ms()-last_retry)>=1000u){rfid_reader_status=rc522_init();last_retry=port_time_ms();}
        rfid_stack_words=uxTaskGetStackHighWaterMark(NULL);
        vTaskDelay(pdMS_TO_TICKS(APP_RFID_POLL_MS));
    }
}
app_result_t task_rfid_create(void) {
    return xTaskCreate(run,"RFID",APP_RFID_STACK_WORDS,NULL,APP_RFID_PRIORITY,NULL)==pdPASS?APP_OK:APP_NOT_READY;
}
