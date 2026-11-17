/* Nguyen Huu Anh: persistent configuration, single writer = task_rfid. */
#include "config_store.h"
#include "internal_flash.h"
#include "initial_config.h"
#include <string.h>
#define COMMIT 0xa5u
#define CRC_OFFSET 155u
#define MARKER_OFFSET 159u
bool app_uid_valid(const app_uid_t *u) {return u&&(u->length==4||u->length==7||u->length==10);}
bool app_uid_equal(const app_uid_t *a,const app_uid_t *b) {
    return app_uid_valid(a)&&app_uid_valid(b)&&a->length==b->length&&!memcmp(a->bytes,b->bytes,a->length);
}
bool app_pin_valid(const app_pin_t *p) {
    if(!p||p->digits[APP_PIN_DIGITS])return false;
    for(unsigned i=0;i<APP_PIN_DIGITS;i++)if(p->digits[i]<'0'||p->digits[i]>'9')return false;
    return true;
}
void app_secure_clear(void *p,size_t n) {volatile unsigned char *v=p;while(n--)*v++=0;}
static uint32_t get32(const uint8_t *p) {return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void put32(uint8_t *p,uint32_t v) {for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(v>>(8*i));}
static uint32_t crc32(const uint8_t *p,size_t n) {
    uint32_t c=UINT32_MAX;
    while(n--){c^=*p++;for(unsigned b=0;b<8;b++)c=(c>>1)^((c&1)?0xedb88320u:0u);}
    return ~c;
}
static bool valid(const app_config_t *c) {
    if(!c||!app_pin_valid(&c->user_pin)||!app_pin_valid(&c->admin_pin)||!memcmp(c->user_pin.digits,c->admin_pin.digits,APP_PIN_DIGITS))return false;
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++)if(c->slots[i].used){
        if(!app_uid_valid(&c->slots[i].uid))return false;
        for(unsigned j=0;j<i;j++)if(c->slots[j].used&&app_uid_equal(&c->slots[i].uid,&c->slots[j].uid))return false;
    }
    return true;
}
static void encode(uint8_t *r,const app_config_t *c,uint32_t seq) {
    memset(r,0,CONFIG_RECORD_BYTES);memcpy(r,"RF3!",4);r[4]=1;put32(r+8,seq);
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++)if(c->slots[i].used){
        unsigned at=12+12*i;r[at]=1;r[at+1]=c->slots[i].uid.length;
        memcpy(r+at+2,c->slots[i].uid.bytes,c->slots[i].uid.length);
    }
    memcpy(r+132,c->user_pin.digits,6);memcpy(r+138,c->admin_pin.digits,6);
    put32(r+CRC_OFFSET,crc32(r,CRC_OFFSET));r[MARKER_OFFSET]=COMMIT;
}
static bool decode(const uint8_t *r,app_config_t *c,uint32_t *seq) {
    if(memcmp(r,"RF3!",4)||r[4]!=1||r[MARKER_OFFSET]!=COMMIT||get32(r+CRC_OFFSET)!=crc32(r,CRC_OFFSET))return false;
    if(r[5]||r[6]||r[7])return false;
    for(unsigned i=144;i<155;i++)if(r[i])return false;
    memset(c,0,sizeof *c);
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++){
        unsigned at=12+12*i;if(r[at]>1)return false;
        c->slots[i].used=r[at]!=0;c->slots[i].uid.length=r[at+1];
        memcpy(c->slots[i].uid.bytes,r+at+2,10);
        if(!r[at])for(unsigned j=1;j<12;j++)if(r[at+j])return false;
    }
    memcpy(c->user_pin.digits,r+132,6);memcpy(c->admin_pin.digits,r+138,6);*seq=get32(r+8);
    return valid(c);
}
app_result_t config_store_load(config_store_t *s) {
    if(!s)return APP_INVALID_INPUT;
    app_secure_clear(s,sizeof *s);uint8_t r[CONFIG_RECORD_BYTES];
    app_config_t configs[2];uint32_t seq[2]={0};bool good[2]={false};app_result_t result=APP_NOT_READY;
    bool blank=true;
    for(unsigned b=0;b<2;b++){
        if(flash_bank_read((uint8_t)b,0,r,sizeof r)!=APP_OK){result=APP_IO_ERROR;goto done;}
        good[b]=decode(r,&configs[b],&seq[b]);
        for(size_t i=0;i<sizeof r;i++)if(r[i]!=0xffu)blank=false;
    }
    if(good[0]||good[1]){
        uint8_t b=0;
        if(!good[0]||(good[1]&&(uint32_t)(seq[1]-seq[0])!=0u&&(uint32_t)(seq[1]-seq[0])<0x80000000u))b=1;
        s->config=configs[b];s->revision=seq[b];s->bank=b;s->ready=true;result=APP_OK;
    }else{
        /* A blank record prefix alone does not prove that the whole page is erased. */
        for(uint8_t bank=0;bank<2u&&blank;bank++){
            for(size_t off=CONFIG_RECORD_BYTES;off<CONFIG_BANK_STRIDE;off+=sizeof r){
                size_t count=CONFIG_BANK_STRIDE-off;
                if(count>sizeof r)count=sizeof r;
                if(flash_bank_read(bank,off,r,count)!=APP_OK){result=APP_IO_ERROR;goto done;}
                for(size_t i=0;i<count;i++)if(r[i]!=0xffu)blank=false;
            }
        }
        result=blank?APP_NOT_READY:APP_STORAGE_CORRUPT;
    }
done:
    app_secure_clear(r,sizeof r);app_secure_clear(configs,sizeof configs);return result;
}
static app_result_t commit(config_store_t *s,const app_config_t *c,uint8_t bank,uint32_t seq) {
    uint8_t r[CONFIG_RECORD_BYTES],check[CONFIG_RECORD_BYTES];
    encode(r,c,seq);app_result_t result=APP_IO_ERROR;
    /* Erase only inactive page. Program first 158 bytes; final halfword contains
       last CRC byte and commit marker. Never rewrite a programmed halfword. */
    if(flash_bank_erase(bank)!=APP_OK)goto done;
    if(flash_bank_program(bank,0,r,158)!=APP_OK)goto done;
    if(flash_bank_read(bank,0,check,158)!=APP_OK||memcmp(r,check,158))goto done;
    if(flash_bank_program(bank,158,r+158,2)!=APP_OK)goto done;
    if(flash_bank_read(bank,0,check,sizeof check)!=APP_OK||memcmp(r,check,sizeof r))goto done;
    s->config=*c;s->bank=bank;s->revision=seq;s->ready=true;result=APP_OK;
done:
    if(result!=APP_OK)s->ready=false; /* Outcome may be uncertain: force reload. */
    app_secure_clear(r,sizeof r);app_secure_clear(check,sizeof check);return result;
}
app_result_t config_store_save(config_store_t *s,const app_config_t *c) {
    if(!s||!valid(c))return APP_INVALID_INPUT;
    if(!s->ready)return APP_NOT_READY;
    return commit(s,c,(uint8_t)(1u-s->bank),s->revision+1u);
}
app_result_t config_store_provision(config_store_t *s,const app_config_t *c) {
    if(!s||!valid(c))return APP_INVALID_INPUT;
    app_result_t r=config_store_load(s);
    if(r==APP_OK)return APP_DENIED;
    if(r!=APP_NOT_READY)return r;
    return commit(s,c,0,1);
}

app_result_t config_store_initialize(config_store_t *s) {
    app_result_t result=config_store_load(s);
    if(result!=APP_NOT_READY)return result;
    return commit(s,&app_initial_config,0,1);
}
