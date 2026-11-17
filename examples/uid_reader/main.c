/* Optional real-card UID reader, does not write configuration or open the lock. */
#include "board.h"
#include "device_api.h"
#include "ssd1306.h"
#include "FreeRTOS.h"
#include "task.h"
static void run(void *arg) {
    (void)arg;
    const char hex[]="0123456789ABCDEF";
    for(;;){
        bool display_ok=oled_init(300)==APP_OK;
        if(!display_ok){vTaskDelay(pdMS_TO_TICKS(1000));continue;}
        ssd1306_text(0,0,"SCAN CARD UID");ssd1306_text(2,0,"LOCK: CLOSED");
        if(ssd1306_flush(300)!=APP_OK){vTaskDelay(pdMS_TO_TICKS(1000));continue;}
        if(rc522_init()!=APP_OK){vTaskDelay(pdMS_TO_TICKS(1000));continue;}
        for(;;){
            app_uid_t uid={0};bool present=false;
            app_result_t r=rc522_poll(&uid,&present,150);
            if(r!=APP_OK&&r!=APP_NO_EVENT)break;
            if(r==APP_OK&&present&&uid.length<=APP_UID_MAX_BYTES){
                char text[APP_UID_MAX_BYTES*2+1];
                for(unsigned i=0;i<uid.length;i++){text[2*i]=hex[uid.bytes[i]>>4];text[2*i+1]=hex[uid.bytes[i]&15u];}
                text[2*uid.length]=0;
                ssd1306_clear();ssd1306_text(0,0,"CARD UID (HEX)");ssd1306_text(2,0,"LOCK: CLOSED");ssd1306_text(4,0,text);
                if(ssd1306_flush(300)!=APP_OK)break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
int main(void){
    if(board_init()!=APP_OK)board_assert_failed();
    lock_output_init_closed();
    if(board_bus_mutex_init()!=APP_OK||xTaskCreate(run,"UID",768,NULL,1,NULL)!=pdPASS)board_assert_failed();
    vTaskStartScheduler();board_assert_failed();
}
