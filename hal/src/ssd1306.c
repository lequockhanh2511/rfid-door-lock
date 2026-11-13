#include "ssd1306.h"
#include "device_api.h"
#include "driver_port.h"
#include <string.h>
#include "font5x7.inc"
static uint8_t frame[SSD1306_FRAME_BYTES];
static bool ready;
void ssd1306_clear(void) {memset(frame,0,sizeof frame);}
const uint8_t *ssd1306_framebuffer(void) {return frame;}
app_result_t ssd1306_text(uint8_t row,uint8_t col,const char *text) {
    if(!text||row>=8||col>=21)return APP_INVALID_INPUT;
    for(;*text&&col<21;text++,col++) {
        char ch=*text;if(ch>='a'&&ch<='z')ch=(char)(ch-'a'+'A');
        const uint8_t *g=NULL;
        for(size_t i=0;i<sizeof(font)/sizeof(font[0]);i++)if(font[i].ch==ch)g=font[i].col;
        if(!g)for(size_t i=0;i<sizeof(font)/sizeof(font[0]);i++)if(font[i].ch=='?')g=font[i].col;
        size_t offset=(size_t)row*128u+(size_t)col*6u;
        memcpy(frame+offset,g,5);frame[offset+5]=0;
    }
    return APP_OK;
}
static app_result_t flush(uint32_t start,uint32_t budget) {
    for(uint8_t p=0;p<8;p++) {
        uint8_t command[]={0x00,(uint8_t)(0xb0|p),0x00,0x10};
        uint32_t left=driver_remaining(start,budget);if(!left)return APP_TIMEOUT;
        app_result_t s=port_i2c_write(SSD1306_ADDRESS,command,sizeof command,left);
        if(s!=APP_OK)return s;
        /* Limit each bus hold to 16 data bytes (~1.6 ms @100kHz). */
        for(unsigned x=0;x<128;x+=16) {
            uint8_t packet[17];packet[0]=0x40;memcpy(packet+1,frame+(size_t)p*128+x,16);
            left=driver_remaining(start,budget);if(!left)return APP_TIMEOUT;
            s=port_i2c_write(SSD1306_ADDRESS,packet,sizeof packet,left);if(s!=APP_OK)return s;
        }
    }
    return APP_OK;
}
app_result_t ssd1306_flush(uint32_t t) {
    if(!driver_timeout_valid(t))return APP_INVALID_INPUT;
    if(!ready)return APP_NOT_READY;
    return flush(port_time_ms(),t);
}
app_result_t oled_init(uint32_t t) {
    ready=false;if(!driver_timeout_valid(t))return APP_INVALID_INPUT;
    uint32_t start=port_time_ms();
    /* 128x64 panel, internal charge pump, page addressing mode. */
    const uint8_t init[]={0x00,0xae,0xd5,0x80,0xa8,0x3f,0xd3,0x00,0x40,
       0x8d,0x14,0x20,0x02,0xa1,0xc8,0xda,0x12,0x81,0x7f,
       0xd9,0xf1,0xdb,0x40,0xa4,0xa6,0x2e};
    app_result_t s=port_i2c_write(SSD1306_ADDRESS,init,sizeof init,t);if(s!=APP_OK)return s;
    ssd1306_clear();s=flush(start,t);if(s!=APP_OK)return s;
    uint32_t left=driver_remaining(start,t);if(!left)return APP_TIMEOUT;
    const uint8_t on[]={0x00,0xaf};s=port_i2c_write(SSD1306_ADDRESS,on,sizeof on,left);
    if(s==APP_OK)ready=true;
    return s;
}
static void draw_number(uint8_t row,uint8_t col,uint32_t number,unsigned min_digits) {
    char reversed[11],text[11];unsigned n=0;
    do { reversed[n++]=(char)('0'+number%10u);number/=10u; } while(number&&n<10);
    while(n<min_digits&&n<10)reversed[n++]='0';
    for(unsigned i=0;i<n;i++)text[i]=reversed[n-1-i];
    text[n]=0;ssd1306_text(row,col,text);
}
app_result_t oled_present(const app_ui_snapshot_t *v,uint32_t t) {
    static const char *const states[]={"BOOT","SETUP REQUIRED","READY","ENTER PIN","UNLOCKED",
     "ADMIN PIN","ADMIN MENU","SCAN NEW CARD","DELETE SLOT","CONFIRM DELETE",
     "CARD LIST","CHANGE PIN","PLEASE WAIT","ERROR"};
    if(!v||(unsigned)v->state>=sizeof states/sizeof states[0]||v->entered_digits>APP_PIN_DIGITS||
       v->card_id>APP_CARD_CAPACITY||v->used_count>APP_CARD_CAPACITY)return APP_INVALID_INPUT;
    ssd1306_clear();ssd1306_text(0,0,"RFID DOOR LOCK");ssd1306_text(1,0,states[v->state]);
    ssd1306_text(2,0,v->unlocked?"LOCK: OPEN":"LOCK: CLOSED");
    char line[22];ssd1306_text(3,0,"CARD:00 TOTAL:00");
    draw_number(3,5,v->card_id,2);draw_number(3,14,v->used_count,2);
    if(v->state==APP_STATE_PIN_ENTRY||v->state==APP_STATE_ADMIN_AUTH||v->state==APP_STATE_CHANGE_PIN){
        memset(line,'*',v->entered_digits);line[v->entered_digits]=0;ssd1306_text(4,0,line);
    } else {
        ssd1306_text(4,0,"RESULT:");draw_number(4,7,(uint32_t)v->result,1);
    }
    if(v->state==APP_STATE_ADMIN_MENU){ssd1306_text(5,0,"1 ADD 2 DEL 3 LIST");ssd1306_text(6,0,"4 USER PIN 5 ADMIN");}
    else if(v->unlocked){ssd1306_text(5,0,"LEFT MS:");draw_number(5,8,v->remaining_ms,1);}
    ssd1306_text(7,0,"# OK  * BACK  D EXIT");
    return ssd1306_flush(t);
}
