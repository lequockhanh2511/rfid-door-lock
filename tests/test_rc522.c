#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "driver_port.h"
#include "device_api.h"
#include "rc522_protocol.h"
static uint32_t tick;static uint8_t regs[64],fifo[64],answer[64];static size_t fn,an,ap;
static int stage,uidbytes=4,corrupt,collision,absent,spi_fail,hang;
uint32_t port_time_ms(void){return tick;}
void port_delay_ms(uint32_t n){tick+=n;}
void port_rc522_reset(bool h){if(!h){memset(regs,0,sizeof regs);regs[0x37]=0x92;}}
static void response(void){
 ap=0;an=0;regs[4]=0x30;regs[6]=0;regs[0xc]=0;
 if(hang){regs[4]=0;return;}
 if(fifo[0]==0x52){assert(fn==1);stage=0;if(absent){regs[4]=1;return;}answer[0]=4;answer[1]=0;an=2;}
 else if(fifo[0]==0x50){assert(fn==4&&fifo[2]==0x57&&fifo[3]==0xcd);regs[4]=1;}
 else {
  assert(fifo[0]==0x93+stage*2);
  bool more=uidbytes>(stage*3+4);
  if(fifo[1]==0x20){
   assert(fn==2);answer[0]=more?0x88:(uint8_t)(stage*3+1);
   for(int i=1;i<4;i++)answer[i]=(uint8_t)(stage*3+i+(more?0:1));
   answer[4]=answer[0]^answer[1]^answer[2]^answer[3];if(corrupt==1)answer[4]^=1;
   an=5;if(collision)regs[6]=8;
  }else{
   assert(fn==9&&fifo[1]==0x70);uint16_t crc=rc522_crc_a(fifo,7);
   assert(fifo[7]==(uint8_t)crc&&fifo[8]==(uint8_t)(crc>>8));
   answer[0]=more?4:0;crc=rc522_crc_a(answer,1);answer[1]=(uint8_t)crc;answer[2]=(uint8_t)(crc>>8);
   if(corrupt==2)answer[2]^=1;
   an=3;stage++;
  }
 }
}
app_result_t port_spi_transfer(const uint8_t *tx,uint8_t *rx,size_t n,uint32_t timeout){
 assert(n==2&&timeout>0);if(spi_fail)return APP_IO_ERROR;
 unsigned reg=(tx[0]&0x7e)>>1;rx[0]=rx[1]=0;
 if(tx[0]&0x80){if(reg==9){assert(ap<an);rx[1]=answer[ap++];}else if(reg==0xa)rx[1]=(uint8_t)an;else rx[1]=regs[reg];}
 else if(reg==9){assert(fn<64);fifo[fn++]=tx[1];}
 else if(reg==0xa&&tx[1]==0x80)fn=0;
 else if(reg==4)regs[4]=0;
 else {regs[reg]=tx[1];if(reg==0xd&&(tx[1]&0x80))response();}
 return APP_OK;
}
int main(void){
 uint8_t halt[]={0x50,0};assert(rc522_crc_a(halt,2)==0xcd57);
 assert(rc522_init()==APP_OK);app_uid_t u;bool present;
 for(uidbytes=4;uidbytes<=10;uidbytes+=3){assert(rc522_poll(&u,&present,200)==APP_OK&&present);assert(u.length==uidbytes);for(int i=0;i<uidbytes;i++)assert(u.bytes[i]==i+1);}
 puts("PASS RC522 CRC known vector, WUPA, select cascades 4/7/10 bytes, HALT");
 uidbytes=4;absent=1;assert(rc522_poll(&u,&present,100)==APP_NO_EVENT&&!present);absent=0;
 corrupt=1;assert(rc522_poll(&u,&present,100)==APP_IO_ERROR&&!present);
 corrupt=2;assert(rc522_poll(&u,&present,100)==APP_IO_ERROR&&!present);corrupt=0;
 collision=1;assert(rc522_poll(&u,&present,100)==APP_IO_ERROR);collision=0;
 spi_fail=1;assert(rc522_poll(&u,&present,100)==APP_IO_ERROR);spi_fail=0;
 hang=1;tick=UINT32_MAX-2;assert(rc522_poll(&u,&present,5)==APP_TIMEOUT);
 assert(rc522_poll(&u,&present,0)==APP_INVALID_INPUT);
 puts("PASS RC522 absent, BCC/SAK corruption, collision, SPI failure, bounded wraparound timeout");
}
