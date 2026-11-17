/* Acceptance tests specifically against HTNN04_N02, host/mocked Flash only. */
#include "access_control.h"
#include "rfid_service.h"
#include "fake_flash.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static access_control_t c;
static config_store_t store;
static app_command_t queued;
static uint32_t now;
static bool waiting;
static app_result_t send(const app_command_t *cmd,void *ctx){(void)ctx;assert(!waiting);queued=*cmd;waiting=true;return APP_OK;}
static void reply(void){assert(waiting);app_event_t e;rfid_service_command(&store,&queued,&e);waiting=false;access_control_event(&c,&e,now);}
static void key(char k){now+=10;access_control_key(&c,k,now);}
static void keys(const char *s){while(*s)key(*s++);}
static void advance(uint32_t ms){now+=ms;access_control_tick(&c,now);}
static void admin(const char *pin){key('A');keys(pin);key('#');reply();assert(c.admin&&c.view.state==APP_STATE_ADMIN_MENU&&!c.view.unlocked);}
static void start(uint32_t time){now=time;waiting=false;access_control_init(&c,now,send,NULL);reply();assert(c.view.state==APP_STATE_IDLE);}
static void try_change(char role,const char *old_pin,const char *new_pin){
    key('3');assert(c.view.state==APP_STATE_PIN_ROLE);key(role);
    assert(c.view.state==APP_STATE_CHANGE_PIN&&c.view.pin_stage==0);
    keys(old_pin);key('#');assert(c.view.pin_stage==1&&!waiting);
    keys(new_pin);key('#');assert(c.view.pin_stage==2&&!waiting);
    keys(new_pin);key('#');assert(waiting);reply();
}
int main(void){
    test_flash_reset();app_config_t cfg=test_config();assert(config_store_provision(&store,&cfg)==APP_OK);start(0);
    /* Three completed wrong Admin PINs, blocked until exactly 30 s. */
    for(unsigned i=0;i<3;i++){keys("A000000#");reply();assert(!c.admin&&!c.view.unlocked&&c.view.result==APP_DENIED);}
    assert(c.admin_locked&&c.view.admin_lockout_ms==30000u);advance(29900);key('A');assert(!waiting&&c.view.state==APP_STATE_IDLE&&c.admin_locked);
    advance(90);assert(!c.admin_locked&&c.view.admin_lockout_ms==0);admin(APP_DEMO_ADMIN_PIN);
    puts("PASS OUTLINE 01: Admin PIN fails three times, blocked 30 s, then retry allowed");
    /* 30s from most recent user interaction, including inside submenus. */
    advance(29999);assert(c.admin);advance(1);assert(!c.admin&&c.view.state==APP_STATE_IDLE);
    admin(APP_DEMO_ADMIN_PIN);key('2');advance(20000);assert(c.admin&&c.view.state==APP_STATE_DELETE_INPUT);
    advance(9999);assert(c.admin);advance(1);assert(!c.admin&&c.view.state==APP_STATE_IDLE);
    puts("PASS OUTLINE 02: 30 s idle exits Admin menu and submenus");
    admin(APP_DEMO_ADMIN_PIN);key('3');assert(c.view.state==APP_STATE_PIN_ROLE);key('*');assert(c.view.state==APP_STATE_ADMIN_MENU);
    keys("202#");assert(c.view.state==APP_STATE_DELETE_CONFIRM);key('*');assert(c.admin&&c.view.state==APP_STATE_ADMIN_MENU&&!waiting);
    assert(c.view.used_count==10);keys("202##");reply();assert(c.view.used_count==9);key('1');key('*');assert(c.view.state==APP_STATE_ADMIN_MENU);
    puts("PASS OUTLINE 03: 3 selects PIN change; * cancels, # confirms; delete only on confirmation");
    try_change('1',"000000","111111");assert(c.view.result==APP_DENIED&&!memcmp(store.config.user_pin.digits,APP_DEMO_USER_PIN,7));
    /* Direct SET command without current PIN must also fail (not only UI). */
    app_command_t cmd={0};app_event_t e;cmd.kind=APP_CMD_SET_PIN;cmd.data.credential.role=APP_PIN_USER;memcpy(cmd.data.credential.pin.digits,"111111",7);
    rfid_service_command(&store,&cmd,&e);assert(e.result==APP_INVALID_INPUT);
    try_change('1',APP_DEMO_USER_PIN,APP_DEMO_ADMIN_PIN);assert(c.view.result==APP_INVALID_INPUT);
    try_change('1',APP_DEMO_USER_PIN,"111111");assert(c.view.result==APP_OK);
    try_change('2',APP_DEMO_ADMIN_PIN,"222222");assert(c.view.result==APP_OK);
    assert(config_store_load(&store)==APP_OK&&!memcmp(store.config.user_pin.digits,"111111",7)&&!memcmp(store.config.admin_pin.digits,"222222",7));
    puts("PASS OUTLINE 04: old PIN required, wrong rejected, PINs distinct; user/admin changes persist");
    key('*');assert(!c.admin);keys("111111#");reply();assert(c.view.unlocked);advance(4999);assert(c.view.unlocked);advance(1);assert(!c.view.unlocked);
    puts("PASS OUTLINE 05: 5 s commanded unlock, closes exactly on simulated deadline");
    /* Verify 32-bit wrap safety plus counter reset after correct authentication. */
    start(UINT32_MAX-500u);keys("A000000#");reply();keys("A000000#");reply();admin("222222");key('*');
    keys("A000000#");reply();assert(c.admin_failures==1&&!c.admin_locked);
    keys("A000000#");reply();keys("A000000#");reply();assert(c.admin_locked);advance(29999);assert(c.admin_locked);advance(1);assert(!c.admin_locked);
    puts("PASS OUTLINE 06: good Admin PIN resets failed count; lockout handles tick wrap");
    return 0;
}
