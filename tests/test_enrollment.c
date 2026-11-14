/* End-to-end enrollment with the exact generated RC1 configuration.
 * Physical card events below use TEST_ONLY UID fixtures, never claimed real. */
#include "provision_data.h"
#include "access_control.h"
#include "rfid_service.h"
#include "fake_eeprom.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static access_control_t control;static config_store_t store;
static app_command_t command;static bool pending;static uint32_t now;
static app_result_t send(const app_command_t *c,void *ctx){(void)ctx;assert(!pending);command=*c;pending=true;return APP_OK;}
static void reply(void){app_event_t e;assert(pending);rfid_service_command(&store,&command,&e);pending=false;access_control_event(&control,&e,++now);}
static void keys(const char *s){while(*s){now+=10;access_control_key(&control,*s++,now);}}
static void card(app_uid_t u){app_event_t e={0};e.kind=APP_EVENT_CARD_PRESENT;e.uid=u;e.occurred_ms=++now;access_control_event(&control,&e,now);}
static void reboot(void){pending=false;access_control_init(&control,++now,send,NULL);reply();}
static void close_lock(void){now+=APP_UNLOCK_MS;access_control_tick(&control,now);assert(!control.view.unlocked);}
int main(void){
    _Static_assert(PROVISION_OCCUPIED_SLOTS==8u,"RC1 starts with eight virtual slots");
    app_uid_t first={.length=4,.bytes={0xf0,0x11,0x22,0x33}};
    app_uid_t second={.length=7,.bytes={0xf0,0x44,0x55,0x66,0x77,0x88,0x99}};
    app_uid_t third={.length=10,.bytes={0xf0,1,2,3,4,5,6,7,8,9}};
    test_eeprom_reset();assert(config_store_provision(&store,&provision_config)==APP_OK);
    assert(!store.config.slots[0].used&&!store.config.slots[1].used);
    for(unsigned i=2;i<10;i++)assert(store.config.slots[i].used);
    reboot();assert(control.view.state==APP_STATE_IDLE&&control.view.used_count==8&&control.view.occupied_mask==0x3fc);
    keys("123456#");reply();assert(control.view.unlocked);close_lock();
    card(first);reply();assert(control.view.result==APP_NOT_FOUND&&!control.view.unlocked);
    keys("A654321#");reply();assert(control.admin);
    keys("1");card(first);reply();assert(control.view.card_id==1&&control.view.used_count==9);
    keys("1");card(second);reply();assert(control.view.card_id==2&&control.view.used_count==10);
    keys("1");card(first);reply();assert(control.view.result==APP_DUPLICATE);
    keys("1");card(third);reply();assert(control.view.result==APP_FULL);
    keys("203##");reply();assert(control.view.used_count==9);
    keys("1");card(third);reply();assert(control.view.card_id==3&&control.view.used_count==10);
    keys("4111111#111111#");reply();assert(control.view.result==APP_OK);
    keys("D");reboot();
    card(first);reply();assert(control.view.unlocked&&control.view.card_id==1);close_lock();
    card(second);reply();assert(control.view.unlocked&&control.view.card_id==2);close_lock();
    card(third);reply();assert(control.view.unlocked&&control.view.card_id==3);close_lock();
    keys("123456#");reply();assert(!control.view.unlocked);
    keys("111111#");reply();assert(control.view.unlocked);close_lock();
    unsigned char before[32768];memcpy(before,test_eeprom,sizeof before);
    assert(config_store_provision(&store,&provision_config)==APP_DENIED);
    assert(!memcmp(before,test_eeprom,sizeof before));
    assert(store.config.user_pin.digits[0]=='1'&&store.config.user_pin.digits[1]=='1');
    test_eeprom_reset();test_write_budget=0;
    assert(config_store_provision(&store,&provision_config)==APP_IO_ERROR);
    test_write_budget=-1;reboot();assert(control.view.state==APP_STATE_SETUP_REQUIRED&&!control.view.unlocked);
    puts("PASS RC1 enrollment: eight virtual slots, default PIN, admin add 4/7-byte UID at 01/02, full/duplicate, delete03/add 10-byte UID, reboot/PIN persistence, no overwrite on setup rerun, setup failure stays locked");
}
