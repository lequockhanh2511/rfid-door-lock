#include "access_control.h"
#include "rfid_service.h"
#include "fake_eeprom.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static access_control_t c;static config_store_t store;static app_command_t queued;
static uint32_t now;static bool waiting,queue_full;
static app_result_t send_command(const app_command_t *cmd,void *ctx){
    (void)ctx;if(queue_full)return APP_BUSY;assert(!waiting);queued=*cmd;waiting=true;return APP_OK;
}
static void reply(void){assert(waiting);app_event_t e;rfid_service_command(&store,&queued,&e);waiting=false;access_control_event(&c,&e,now);}
static void key(char k){now+=10;access_control_key(&c,k,now);}
static void keys(const char *s){while(*s)key(*s++);}
static void advance(uint32_t ms){now+=ms;access_control_tick(&c,now);}
static void card(app_uid_t uid){now++;app_event_t e={0};e.kind=APP_EVENT_CARD_PRESENT;e.uid=uid;e.occurred_ms=now;access_control_event(&c,&e,now);}
static void admin(void){keys("A654321#");assert(waiting);reply();assert(c.admin&&c.view.state==APP_STATE_ADMIN_MENU&&!c.view.unlocked);}
static void start(uint32_t time){now=time;waiting=false;queue_full=false;access_control_init(&c,now,send_command,NULL);reply();}
int main(void){
    app_config_t cfg=test_config();test_eeprom_reset();assert(config_store_provision(&store,&cfg)==APP_OK);start(0);
    assert(c.view.state==APP_STATE_IDLE&&c.view.used_count==10&&!c.view.unlocked);
    app_uid_t stranger={.length=4,.bytes={0x77,0x88,0x99,0xaa}};
    card(stranger);reply();assert(c.view.result==APP_NOT_FOUND&&!c.view.unlocked);
    keys("000000#");reply();assert(c.view.result==APP_DENIED&&!c.view.unlocked);
    keys("123456#");reply();assert(c.view.unlocked&&c.view.card_id==0);
    key('A');assert(!c.admin&&c.view.state==APP_STATE_UNLOCKED);
    advance(4900);card(cfg.slots[1].uid);reply();assert(c.view.card_id==2&&c.view.unlocked);
    advance(4999);assert(c.view.unlocked);advance(1);assert(!c.view.unlocked&&c.view.state==APP_STATE_IDLE);
    keys("123*");assert(c.length==2);key('D');assert(c.length==0&&c.view.state==APP_STATE_IDLE);
    keys("12");advance(APP_INPUT_TIMEOUT_MS);assert(c.length==0&&c.view.state==APP_STATE_IDLE);
    keys("A000000#");reply();assert(!c.admin);key('1');assert(c.view.state==APP_STATE_PIN_ENTRY);key('D');
    admin();card(cfg.slots[0].uid);assert(!waiting&&!c.view.unlocked);
    key('1');card(stranger);reply();assert(c.view.result==APP_FULL&&c.admin);
    keys("202#");assert(c.view.state==APP_STATE_DELETE_CONFIRM&&c.view.selected_id==2);key('#');reply();
    assert(c.view.used_count==9&&c.view.occupied_mask==0x3fd);
    key('1');card(stranger);reply();assert(c.view.result==APP_OK&&c.view.card_id==2&&c.view.used_count==10);
    key('1');card(stranger);reply();assert(c.view.result==APP_DUPLICATE);
    key('3');assert(c.view.selected_id==1);key('6');assert(c.view.selected_id==2);key('4');assert(c.view.selected_id==1);key('D');
    keys("4111111#222222#");assert(c.view.result==APP_INVALID_INPUT&&!waiting&&!c.confirm_pin);
    keys("111111#111111#");reply();assert(c.view.result==APP_OK&&c.admin);
    key('D');keys("123456#");reply();assert(!c.view.unlocked);keys("111111#");reply();assert(c.view.unlocked);
    advance(5000);start(now);card(stranger);reply();assert(c.view.unlocked&&c.view.card_id==2);advance(5000);
    admin();advance(APP_ADMIN_TIMEOUT_MS);assert(!c.admin&&c.view.state==APP_STATE_IDLE);
    /* A card sampled inside Admin just before its expiry cannot open after exit. */
    admin();uint32_t expiry=c.last_admin+APP_ADMIN_TIMEOUT_MS;
    app_event_t expired={0};expired.kind=APP_EVENT_CARD_PRESENT;expired.uid=cfg.slots[0].uid;expired.occurred_ms=expiry-1;
    now=expiry;access_control_event(&c,&expired,now);assert(!waiting&&!c.admin&&!c.view.unlocked);
    /* Old card queued before ADD mode must not be enrolled. */
    admin();uint32_t old=now;key('1');app_event_t event={0};event.kind=APP_EVENT_CARD_PRESENT;event.uid=stranger;event.occurred_ms=old;
    access_control_event(&c,&event,now);assert(!waiting);
    event.occurred_ms=c.mode_at;access_control_event(&c,&event,now);assert(!waiting);key('D');key('D');
    event.occurred_ms=now-501;access_control_event(&c,&event,now);assert(!waiting);
    /* PIN entry interrupted by a valid card; secrets are wiped. */
    keys("11");card(stranger);assert(c.length==0);reply();assert(c.view.unlocked);advance(5000);
    /* Cancel an outstanding authentication; its late success cannot open. */
    keys("111111#");key('D');reply();assert(!c.view.unlocked&&!c.admin);
    keys("A654321#");key('D');reply();assert(!c.admin);
    /* Wrong request ID/command cannot apply a response. */
    keys("111111#");event=(app_event_t){0};event.kind=APP_EVENT_COMMAND_RESULT;event.request_id=c.pending_id+1;event.command=APP_CMD_VERIFY_PIN;
    access_control_event(&c,&event,now);assert(c.pending&&!c.view.unlocked);
    reply();advance(5000);
    /* Queue backpressure does not pretend a command succeeded. */
    queue_full=true;keys("111111#");assert(!waiting&&!c.view.unlocked&&c.view.result==APP_BUSY);queue_full=false;key('D');
    /* Timeout closes fail-safe; late result is discarded; explicit reload recovers. */
    keys("111111#");advance(3000);assert(c.view.state==APP_STATE_ERROR&&!c.view.unlocked);reply();assert(!c.view.unlocked);
    key('D');reply();assert(c.view.state==APP_STATE_IDLE);
    /* A submitted delete still saves after D, but never retains admin authority. */
    admin();keys("202##");assert(waiting);key('D');reply();assert(!c.admin&&c.view.used_count==9);
    admin();key('1');test_write_budget=0;card(stranger);reply();assert(c.view.state==APP_STATE_ERROR&&!c.admin&&!c.view.unlocked);
    test_write_budget=-1;key('D');reply();assert(c.view.used_count==9);
    /* 32-bit clock rollover: unlock and entry deadlines use unsigned elapsed. */
    start(UINT32_MAX-1000u);card(cfg.slots[0].uid);reply();advance(4999);assert(c.view.unlocked);advance(1);assert(!c.view.unlocked);
    keys("12");advance(20000);assert(c.view.state==APP_STATE_IDLE);
    /* No automatic default PIN on missing/corrupt storage. */
    test_eeprom_reset();start(0);assert(c.view.state==APP_STATE_SETUP_REQUIRED&&!c.view.unlocked);
    keys("123456#");assert(!waiting&&!c.view.unlocked);
    puts("PASS control/service integration: RFID/PIN/admin, 5s extension/relock, 20s/60s timeout, delete/add02, PIN confirmation, reboot, queue BUSY, cancel/late/stale events, storage fault, tick wrap, blank EEPROM");
}
