/* M2 diagnostic firmware only. No authentication or Admin Mode. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device_api.h"
#include "keypad.h"
#include "eeprom24.h"
/* Watch these globals in debugger. No fake test results, no UART dependency. */
volatile app_result_t diag_rfid_init=APP_NOT_READY,diag_rfid_poll=APP_NOT_READY;
volatile app_result_t diag_oled=APP_NOT_READY,diag_eeprom=APP_NOT_READY;
volatile uint32_t diag_card_reads,diag_key_events,diag_keypad_errors;
volatile char diag_last_key;
volatile app_uid_t diag_last_uid;
volatile uint32_t diag_control_stack_words,diag_rfid_stack_words,diag_ui_stack_words;
volatile uint32_t diag_free_heap;
static void control(void *arg) {
    (void)arg;keypad_state_t state;keypad_state_init(&state,HAL_GetTick());
    TickType_t last=xTaskGetTickCount();
    for(;;){
        uint16_t mask;char key;
        if(keypad_read_matrix(&mask)==APP_OK){
            if(keypad_update(&state,mask,HAL_GetTick(),&key)==APP_OK){diag_last_key=key;diag_key_events++;}
        }else diag_keypad_errors++;
        lock_output_set(false); /* M2 never grants access. */
        diag_control_stack_words=uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&last,pdMS_TO_TICKS(APP_KEYPAD_SCAN_MS));
    }
}
static void rfid(void *arg) {
    (void)arg;diag_rfid_init=rc522_init();uint32_t last_storage=HAL_GetTick();
    for(;;){
        if(diag_rfid_init==APP_OK){
            app_uid_t uid;bool present;
            diag_rfid_poll=rc522_poll(&uid,&present,200);
            if(diag_rfid_poll==APP_OK&&present){diag_last_uid=uid;diag_card_reads++;}
            /* A protocol failure may leave PICC active: reinitialize RF field. */
            else if(diag_rfid_poll!=APP_NO_EVENT)diag_rfid_init=APP_NOT_READY;
        }else diag_rfid_init=rc522_init();
        if((uint32_t)(HAL_GetTick()-last_storage)>=1000u){
            uint8_t bytes[16];diag_eeprom=eeprom24_read(0,bytes,sizeof bytes,100);
            last_storage=HAL_GetTick(); /* non-destructive diagnostic; no provisioning */
        }
        diag_rfid_stack_words=uxTaskGetStackHighWaterMark(NULL);
        vTaskDelay(pdMS_TO_TICKS(APP_RFID_POLL_MS));
    }
}
static void ui(void *arg) {
    (void)arg;diag_oled=oled_init(300);
    for(;;){
        if(diag_oled!=APP_OK)diag_oled=oled_init(300);
        else {
            app_ui_snapshot_t v={0};v.state=APP_STATE_SETUP_REQUIRED;
            v.result=diag_eeprom;diag_oled=oled_present(&v,300);
        }
        diag_ui_stack_words=uxTaskGetStackHighWaterMark(NULL);
        diag_free_heap=(uint32_t)xPortGetFreeHeapSize();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
int main(void) {
    if(board_init()!=APP_OK)board_assert_failed();
    lock_output_init_closed();
    if(board_bus_mutex_init()!=APP_OK)board_assert_failed();
    if(xTaskCreate(control,"DIAG_KEYS",APP_CONTROL_STACK_WORDS,NULL,APP_CONTROL_PRIORITY,NULL)!=pdPASS||
       xTaskCreate(rfid,"DIAG_RFID",APP_RFID_STACK_WORDS,NULL,APP_RFID_PRIORITY,NULL)!=pdPASS||
       xTaskCreate(ui,"DIAG_UI",APP_UI_STACK_WORDS,NULL,APP_UI_PRIORITY,NULL)!=pdPASS)board_assert_failed();
    vTaskStartScheduler();board_assert_failed();
}
