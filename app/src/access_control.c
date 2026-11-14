/* Ha Xuan Khai: state and deadlines. No peripheral/FreeRTOS calls here. */
#include "access_control.h"
#include "config_store.h"
#include <string.h>
#define REQUEST_TIMEOUT_MS 3000u
#define CARD_MAX_AGE_MS 500u
static void clear_input(access_control_t *c) {
    app_secure_clear(c->input,sizeof c->input);c->length=0;c->view.entered_digits=0;
}
static void clear_secrets(access_control_t *c) {
    clear_input(c);app_secure_clear(&c->new_pin,sizeof c->new_pin);c->confirm_pin=false;c->view.confirm_pin=false;
}
static void enter(access_control_t *c,app_state_t state,uint32_t now) {
    clear_secrets(c);c->view.state=state;c->mode_at=now;c->last_input=now;c->view.selected_id=0;
}
static void leave(access_control_t *c,uint32_t now) {
    enter(c,c->admin?APP_STATE_ADMIN_MENU:(c->view.unlocked?APP_STATE_UNLOCKED:APP_STATE_IDLE),now);
}
static void error(access_control_t *c,app_result_t result,uint32_t now) {
    c->admin=false;c->view.unlocked=false;c->view.remaining_ms=0;c->view.result=result;
    enter(c,result==APP_NOT_READY?APP_STATE_SETUP_REQUIRED:APP_STATE_ERROR,now);
}
static void unlock(access_control_t *c,uint32_t now) {
    c->admin=false;c->view.unlocked=true;c->unlock_at=now;c->view.remaining_ms=APP_UNLOCK_MS;
    enter(c,APP_STATE_UNLOCKED,now);
}
static void submit(access_control_t *c,app_command_t *cmd,uint32_t now) {
    if(c->pending){c->view.result=APP_BUSY;return;}
    if(++c->next_id==0)++c->next_id;
    cmd->request_id=c->next_id;
    app_result_t r=c->send(cmd,c->send_ctx);
    if(r!=APP_OK){c->view.result=r;app_secure_clear(cmd,sizeof *cmd);return;}
    c->pending=true;c->cancelled=false;c->pending_id=cmd->request_id;c->pending_kind=cmd->kind;
    c->pending_role=cmd->data.credential.role;c->pending_at=now;
    enter(c,APP_STATE_WAIT_RESULT,now);c->view.result=APP_OK;
    app_secure_clear(cmd,sizeof *cmd);
}
void access_control_init(access_control_t *c,uint32_t now,control_send_fn send,void *ctx) {
    memset(c,0,sizeof *c);c->send=send;c->send_ctx=ctx;c->last_admin=now;
    c->view.state=APP_STATE_BOOT;app_command_t cmd={0};cmd.kind=APP_CMD_GET_STATUS;submit(c,&cmd,now);
    if(!c->pending)error(c,c->view.result,now);
}
void access_control_tick(access_control_t *c,uint32_t now) {
    if(c->view.unlocked){
        uint32_t elapsed=now-c->unlock_at;
        if(elapsed>=APP_UNLOCK_MS){c->view.unlocked=false;c->view.remaining_ms=0;if(c->view.state==APP_STATE_UNLOCKED)enter(c,APP_STATE_IDLE,now);}
        else c->view.remaining_ms=APP_UNLOCK_MS-elapsed;
    }
    if(c->pending&&(uint32_t)(now-c->pending_at)>=REQUEST_TIMEOUT_MS){
        c->pending=false;c->cancelled=true;error(c,APP_TIMEOUT,now);return;
    }
    if(c->admin&&(uint32_t)(now-c->last_admin)>=APP_ADMIN_TIMEOUT_MS){
        c->admin=false;c->cancelled=c->pending;
        if(!c->pending)enter(c,APP_STATE_IDLE,now);
        else clear_secrets(c);
    }
    app_state_t s=c->view.state;
    if((s==APP_STATE_PIN_ENTRY||s==APP_STATE_ADMIN_AUTH||s==APP_STATE_DELETE_INPUT||
        s==APP_STATE_DELETE_CONFIRM||s==APP_STATE_CHANGE_PIN)&&(uint32_t)(now-c->last_input)>=APP_INPUT_TIMEOUT_MS){
        c->view.result=APP_TIMEOUT;leave(c,now);
    }
}
void access_control_key(access_control_t *c,char key,uint32_t now) {
    access_control_tick(c,now);c->last_input=now;
    if(c->admin)c->last_admin=now;
    if(c->pending){if(key=='D'){c->cancelled=true;c->admin=false;clear_secrets(c);}return;}
    app_state_t s=c->view.state;
    if(s==APP_STATE_ERROR||s==APP_STATE_SETUP_REQUIRED){
        if(key=='D'){app_command_t cmd={0};cmd.kind=APP_CMD_GET_STATUS;submit(c,&cmd,now);}return;
    }
    if(key=='D'){
        if(s==APP_STATE_ADMIN_MENU)c->admin=false;
        leave(c,now);c->view.result=APP_OK;return;
    }
    if(s==APP_STATE_IDLE||s==APP_STATE_UNLOCKED){
        if(key=='A'&&!c->view.unlocked){enter(c,APP_STATE_ADMIN_AUTH,now);c->pin_role=APP_PIN_ADMIN;}
        else if(key>='0'&&key<='9'){enter(c,APP_STATE_PIN_ENTRY,now);c->pin_role=APP_PIN_USER;}
        else return;
        c->view.result=APP_OK;s=c->view.state;if(key=='A')return;
    }
    if(s==APP_STATE_ADMIN_MENU){
        c->view.result=APP_OK;
        if(key=='1')enter(c,APP_STATE_ADD_WAIT_CARD,now);
        else if(key=='2')enter(c,APP_STATE_DELETE_INPUT,now);
        else if(key=='3'){enter(c,APP_STATE_LIST_CARDS,now);c->view.selected_id=1;}
        else if(key=='4'||key=='5'){
            enter(c,APP_STATE_CHANGE_PIN,now);c->pin_role=key=='4'?APP_PIN_USER:APP_PIN_ADMIN;c->view.pin_role=c->pin_role;
        }return;
    }
    if(s==APP_STATE_LIST_CARDS){
        if(key=='4')c->view.selected_id=c->view.selected_id==1?10:(uint8_t)(c->view.selected_id-1);
        if(key=='6')c->view.selected_id=c->view.selected_id==10?1:(uint8_t)(c->view.selected_id+1);
        return;
    }
    if(s==APP_STATE_DELETE_CONFIRM){
        if(key=='#'){app_command_t cmd={0};cmd.kind=APP_CMD_DELETE_CARD;cmd.data.card_id=c->view.selected_id;submit(c,&cmd,now);}
        return;
    }
    if(s!=APP_STATE_PIN_ENTRY&&s!=APP_STATE_ADMIN_AUTH&&s!=APP_STATE_DELETE_INPUT&&s!=APP_STATE_CHANGE_PIN)return;
    unsigned limit=s==APP_STATE_DELETE_INPUT?2:APP_PIN_DIGITS;
    if(key=='*'){if(c->length)c->input[--c->length]=0;}
    else if(key>='0'&&key<='9'){
        if(c->length<limit){c->input[c->length++]=key;c->input[c->length]=0;}
    }else if(key=='#'){
        if(c->length!=limit){c->view.result=APP_INVALID_INPUT;return;}
        if(s==APP_STATE_DELETE_INPUT){
            unsigned id=(unsigned)(c->input[0]-'0')*10u+(unsigned)(c->input[1]-'0');
            if(id<1||id>10){c->view.result=APP_INVALID_INPUT;return;}
            enter(c,APP_STATE_DELETE_CONFIRM,now);c->view.selected_id=(uint8_t)id;return;
        }
        app_command_t cmd={0};cmd.data.credential.role=c->pin_role;
        memcpy(cmd.data.credential.pin.digits,c->input,APP_PIN_DIGITS+1);
        if(s==APP_STATE_CHANGE_PIN){
            if(!c->confirm_pin){c->new_pin=cmd.data.credential.pin;clear_input(c);c->confirm_pin=true;c->view.confirm_pin=true;app_secure_clear(&cmd,sizeof cmd);return;}
            if(memcmp(c->new_pin.digits,c->input,APP_PIN_DIGITS)){
                clear_secrets(c);c->view.result=APP_INVALID_INPUT;app_secure_clear(&cmd,sizeof cmd);return;
            }
            cmd.kind=APP_CMD_SET_PIN;
        }else cmd.kind=APP_CMD_VERIFY_PIN;
        submit(c,&cmd,now);return;
    }
    c->view.entered_digits=c->length;
    if(s==APP_STATE_DELETE_INPUT)c->view.selected_id=c->length?(uint8_t)(c->input[0]-'0'):0;
    if(s==APP_STATE_DELETE_INPUT&&c->length==2)c->view.selected_id=(uint8_t)((c->input[0]-'0')*10+c->input[1]-'0');
}
void access_control_event(access_control_t *c,const app_event_t *e,uint32_t now) {
    access_control_tick(c,now);
    if(e->kind==APP_EVENT_STORAGE_FAULT){c->pending=false;error(c,e->result,now);return;}
    if(e->kind==APP_EVENT_CARD_PRESENT){
        if(c->pending||(uint32_t)(now-e->occurred_ms)>CARD_MAX_AGE_MS)return;
        app_command_t cmd={0};cmd.data.uid=e->uid;
        if(c->admin&&c->view.state==APP_STATE_ADD_WAIT_CARD){
            if((uint32_t)(e->occurred_ms-c->mode_at)>=0x80000000u)return;
            cmd.kind=APP_CMD_ADD_CARD;c->last_admin=now;
        }else if(!c->admin&&(c->view.state==APP_STATE_IDLE||c->view.state==APP_STATE_PIN_ENTRY||c->view.state==APP_STATE_UNLOCKED))cmd.kind=APP_CMD_LOOKUP_CARD;
        else return;
        submit(c,&cmd,now);return;
    }
    if(e->kind!=APP_EVENT_COMMAND_RESULT||!c->pending||e->request_id!=c->pending_id||e->command!=c->pending_kind)return;
    c->pending=false;c->view.result=e->result;c->view.used_count=e->used_count;c->view.occupied_mask=e->occupied_mask;
    if(e->result==APP_IO_ERROR||e->result==APP_NOT_READY||e->result==APP_TIMEOUT){error(c,e->result,now);return;}
    if(c->cancelled){c->admin=false;leave(c,now);return;}
    if(e->command==APP_CMD_GET_STATUS){if(e->result==APP_OK)enter(c,APP_STATE_IDLE,now);else error(c,e->result,now);return;}
    if(e->command==APP_CMD_LOOKUP_CARD){
        if(e->result==APP_OK){c->view.card_id=e->card_id;unlock(c,now);}else leave(c,now);
    }else if(e->command==APP_CMD_VERIFY_PIN){
        if(e->result==APP_OK&&c->pending_role==APP_PIN_ADMIN){c->admin=true;c->last_admin=now;enter(c,APP_STATE_ADMIN_MENU,now);}
        else if(e->result==APP_OK){c->view.card_id=0;unlock(c,now);}
        else leave(c,now);
    }else {if(e->result==APP_OK)c->view.card_id=e->card_id;leave(c,now);}
}
