#include "rfid_service.h"
#include <string.h>
bool card_gate_update(card_gate_t *g,app_result_t r,bool present) {
    if(r==APP_NO_EVENT){if(g->absent<3)g->absent++;if(g->absent==3)g->held=false;return false;}
    g->absent=0;if(r!=APP_OK||!present)return false;
    if(g->held)return false;
    g->held=true;return true;
}
static void stats(const config_store_t *s,app_event_t *e) {
    if(!s->ready)return;
    e->config_revision=s->revision;
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++)if(s->config.slots[i].used){e->used_count++;e->occupied_mask|=(uint16_t)(1u<<i);}
}
void rfid_service_command(config_store_t *s,const app_command_t *c,app_event_t *e) {
    memset(e,0,sizeof *e);e->kind=APP_EVENT_COMMAND_RESULT;e->request_id=c->request_id;e->command=c->kind;
    if(c->kind==APP_CMD_GET_STATUS){e->result=config_store_initialize(s);stats(s,e);return;}
    if(!s->ready){e->result=APP_NOT_READY;return;}
    e->result=APP_INVALID_INPUT;app_config_t next=s->config;bool save=false;
    switch(c->kind){
    case APP_CMD_LOOKUP_CARD: case APP_CMD_ADD_CARD: {
        if(!app_uid_valid(&c->data.uid))break;
        int found=-1,free_slot=-1;
        for(unsigned i=0;i<APP_CARD_CAPACITY;i++){
            if(s->config.slots[i].used){if(app_uid_equal(&s->config.slots[i].uid,&c->data.uid))found=(int)i;}
            else if(free_slot<0)free_slot=(int)i;
        }
        if(found>=0){e->card_id=(uint8_t)(found+1);e->result=c->kind==APP_CMD_ADD_CARD?APP_DUPLICATE:APP_OK;}
        else if(c->kind==APP_CMD_LOOKUP_CARD)e->result=APP_NOT_FOUND;
        else if(free_slot<0)e->result=APP_FULL;
        else {next.slots[free_slot].used=true;next.slots[free_slot].uid=c->data.uid;e->card_id=(uint8_t)(free_slot+1);save=true;}
        break;
    }
    case APP_CMD_DELETE_CARD:
        if(c->data.card_id<1||c->data.card_id>APP_CARD_CAPACITY)break;
        if(!next.slots[c->data.card_id-1].used){e->result=APP_NOT_FOUND;break;}
        memset(&next.slots[c->data.card_id-1],0,sizeof next.slots[0]);e->card_id=c->data.card_id;save=true;break;
    case APP_CMD_VERIFY_PIN: case APP_CMD_SET_PIN: {
        if((unsigned)c->data.credential.role>APP_PIN_ADMIN||!app_pin_valid(&c->data.credential.pin))break;
        app_pin_t *p=c->data.credential.role==APP_PIN_USER?&next.user_pin:&next.admin_pin;
        if(c->kind==APP_CMD_SET_PIN){
            const app_pin_t *other=c->data.credential.role==APP_PIN_USER?&next.admin_pin:&next.user_pin;
            if(!app_pin_valid(&c->data.credential.current_pin))break;
            unsigned diff=0;for(unsigned i=0;i<APP_PIN_DIGITS;i++)diff|=(unsigned char)p->digits[i]^(unsigned char)c->data.credential.current_pin.digits[i];
            if(diff){e->result=APP_DENIED;break;}
            if(!memcmp(other->digits,c->data.credential.pin.digits,APP_PIN_DIGITS))break;
            *p=c->data.credential.pin;save=true;
        }
        else {unsigned diff=0;for(unsigned i=0;i<APP_PIN_DIGITS;i++)diff|=(unsigned char)p->digits[i]^(unsigned char)c->data.credential.pin.digits[i];e->result=diff?APP_DENIED:APP_OK;}
        break;
    }
    default:break;
    }
    if(save)e->result=config_store_save(s,&next);
    app_secure_clear(&next,sizeof next);stats(s,e);
}
