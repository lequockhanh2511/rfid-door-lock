/* Separate, deliberate initial setup image. Normal application never calls this. */
#include "board.h"
#include "device_api.h"
#include "ssd1306.h"
#include "config_store.h"
#include "provision_data.h"
#include "FreeRTOS.h"
#include "task.h"
volatile app_result_t provision_status=APP_BUSY;
volatile uint32_t provision_revision;
static void run(void *arg){
    (void)arg;config_store_t store={0};
#if PROVISION_TEST_ONLY
    /* Test-only build is intentionally unable to write synthetic UIDs to a kit. */
    provision_status=APP_DENIED;
#else
    provision_status=config_store_provision(&store,&provision_config);
#endif
    provision_revision=store.revision;
    if(oled_init(300)==APP_OK){
        ssd1306_clear();ssd1306_text(0,0,"INITIAL SETUP");
#if PROVISION_TEST_ONLY
        ssd1306_text(2,0,"TEST ONLY");ssd1306_text(3,0,"WRITES DISABLED");
#else
        if(provision_status==APP_OK){ssd1306_text(2,0,"SETUP SAVED");ssd1306_text(3,0,"10 SLOTS SAVED");}
        else if(provision_status==APP_DENIED)ssd1306_text(2,0,"CONFIG ALREADY EXISTS");
        else {ssd1306_text(2,0,"SETUP FAILED");ssd1306_text(3,0,"CHECK DEBUG STATUS");}
#endif
        ssd1306_text(5,0,"FLASH APP NEXT");(void)ssd1306_flush(300);
    }
    for(;;){lock_output_set(false);vTaskDelay(pdMS_TO_TICKS(1000));}
}
int main(void){
    if(board_init()!=APP_OK)board_assert_failed();
    lock_output_init_closed();
    if(board_bus_mutex_init()!=APP_OK||xTaskCreate(run,"SETUP",1024,NULL,1,NULL)!=pdPASS)board_assert_failed();
    vTaskStartScheduler();board_assert_failed();
}
