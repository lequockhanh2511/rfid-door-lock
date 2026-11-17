#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "device_api.h"
#include "driver_port.h"
static uint32_t tick;static int fail,bytes,page,col;static bool servo_open;static uint8_t screen[1024];
static void expect_row(const app_ui_snapshot_t *v,unsigned row,const char *text){
 uint8_t expected[128];ssd1306_clear();assert(ssd1306_text((uint8_t)row,0,text)==APP_OK);
 memcpy(expected,ssd1306_framebuffer()+row*128,128);
 assert(oled_present(v,200)==APP_OK);assert(memcmp(screen+row*128,expected,128)==0);
}
uint32_t port_time_ms(void){return tick;}
void port_set_lock_servo(bool opened){servo_open=opened;}
app_result_t port_i2c_write(uint8_t a,const uint8_t *p,size_t n,uint32_t t){
 assert(a==0x3c&&t>0);tick++;
 if(fail)return APP_IO_ERROR;
 if(p[0]==0x40){assert(n==17&&page<8&&col+16<=128);memcpy(screen+page*128+col,p+1,16);col+=16;bytes+=16;}
 else{assert(p[0]==0);if(n==4&&(p[1]&0xf8)==0xb0){page=p[1]&7;col=0;}}
 return APP_OK;
}
int main(void){
 assert(ssd1306_flush(100)==APP_NOT_READY);
 assert(oled_init(200)==APP_OK);assert(bytes==1024);
 for(int i=0;i<1024;i++)assert(screen[i]==0);
 assert(ssd1306_text(0,0,"A")==APP_OK);
 const uint8_t expected[]={0x7e,0x09,0x09,0x09,0x7e,0};
 assert(memcmp(ssd1306_framebuffer(),expected,6)==0);
 assert(ssd1306_text(7,20,"ABCD")==APP_OK);
 assert(ssd1306_text(8,0,"A")==APP_INVALID_INPUT);
 assert(ssd1306_text(0,21,"A")==APP_INVALID_INPUT);
 assert(ssd1306_text(0,0,NULL)==APP_INVALID_INPUT);
 assert(ssd1306_flush(200)==APP_OK);
 assert(memcmp(screen,ssd1306_framebuffer(),1024)==0);
 puts("PASS OLED init, 1024-byte transfer, page/column layout, font A, clipping");
 app_ui_snapshot_t v={0};v.state=APP_STATE_ADMIN_AUTH;v.entered_digits=6;
 assert(oled_present(&v,200)==APP_OK);
 expect_row(&v,5,"******");
 v.card_id=3;v.used_count=8;expect_row(&v,3,"CARD:03 TOTAL:08");
 expect_row(&v,2,"LOCK: CLOSED");
 v.unlocked=true;expect_row(&v,2,"LOCK: OPEN");
 v.card_id=10;v.used_count=10;expect_row(&v,3,"CARD:10 TOTAL:10");
 v.unlocked=false;
 puts("PASS OLED exact card ID/count, OPEN/CLOSED rows and masked PIN");
 v.selected_id=2;v.occupied_mask=0x3ff;v.pin_role=APP_PIN_ADMIN;
 for(unsigned st=0;st<=APP_STATE_PIN_ROLE;st++){
  v.state=(app_state_t)st;
  for(unsigned result=0;result<=APP_STORAGE_CORRUPT;result++){
   v.result=(app_result_t)result;assert(oled_present(&v,200)==APP_OK);
  }
 }
 puts("PASS OLED renders all 14 states and 12 result codes with bounded transfers");
 uint8_t expected_row[128];
 ssd1306_clear();ssd1306_text(6,0,"FULL - DELETE A SLOT");
 memcpy(expected_row,ssd1306_framebuffer()+6*128,128);
 v.state=APP_STATE_ADMIN_MENU;v.result=APP_OK;v.used_count=10;
 assert(oled_present(&v,200)==APP_OK);
 assert(memcmp(screen+6*128,expected_row,128)==0);
 ssd1306_clear();ssd1306_text(4,0,"FLASH DATA CORRUPT");
 memcpy(expected_row,ssd1306_framebuffer()+4*128,128);
 v.state=APP_STATE_ERROR;v.result=APP_STORAGE_CORRUPT;
 assert(oled_present(&v,200)==APP_OK);
 assert(memcmp(screen+4*128,expected_row,128)==0);
 puts("PASS OLED full-capacity notice before an eleventh attempt and distinct corrupt-Flash message");
 v.entered_digits=7;assert(oled_present(&v,200)==APP_INVALID_INPUT);
 fail=1;assert(ssd1306_flush(100)==APP_IO_ERROR);
 lock_output_init_closed();assert(!servo_open);lock_output_set(true);assert(servo_open);lock_output_set(false);assert(!servo_open);
 fail=0;tick=UINT32_MAX-3;assert(ssd1306_flush(5)==APP_TIMEOUT);
 assert(oled_init(0)==APP_INVALID_INPUT);
 puts("PASS OLED validation, bus error, wrap timeout, Servo command independent of display");
}
