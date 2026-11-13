#include "keypad.h"
#include "device_api.h"
#include "driver_port.h"
#include <string.h>
app_result_t keypad_read_matrix(uint16_t *mask) {
    if(!mask)return APP_INVALID_INPUT;
    *mask=0;
    for(uint8_t row=0;row<4;row++)port_keypad_row(row,false);
    for(uint8_t row=0;row<4;row++) {
        port_keypad_row(row,true);port_delay_us(5);
        *mask|=(uint16_t)((port_keypad_columns()&0xfu)<<(row*4u));
        port_keypad_row(row,false);
    }
    return APP_OK;
}
void keypad_state_init(keypad_state_t *s,uint32_t now) {
    if(s){memset(s,0,sizeof(*s));s->changed_at=now;}
}
app_result_t keypad_update(keypad_state_t *s,uint16_t mask,uint32_t now,char *key) {
    static const char keys[]="123A456B789C*0#D";
    if(!s||!key)return APP_INVALID_INPUT;
    *key=0;
    if(mask&&(mask&(uint16_t)(mask-1u)))s->blocked=true;
    if(mask!=s->candidate){s->candidate=mask;s->changed_at=now;return APP_NO_EVENT;}
    if((uint32_t)(now-s->changed_at)<APP_KEYPAD_DEBOUNCE_MS)return APP_NO_EVENT;
    if(!mask){s->latched=false;s->blocked=false;return APP_NO_EVENT;}
    if(s->blocked||s->latched)return APP_NO_EVENT;
    s->latched=true;
    for(unsigned i=0;i<16;i++)if(mask==(uint16_t)(1u<<i)){*key=keys[i];return APP_OK;}
    return APP_NO_EVENT;
}
