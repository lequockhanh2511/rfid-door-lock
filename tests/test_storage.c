#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "eeprom24.h"
#include "driver_port.h"
static uint8_t mem[EEPROM24_BYTES];static uint32_t tick;static int writes,busy,fail,wp,never_ready;
uint32_t port_time_ms(void){return tick;}
void port_delay_ms(uint32_t n){tick+=n;}
app_result_t port_i2c_write(uint8_t a,const uint8_t *p,size_t n,uint32_t t){
 assert(a==0x50&&t>0&&n>=3&&n<=66);if(fail)return APP_IO_ERROR;
 unsigned o=(unsigned)(p[0]*256u+p[1]);assert(o/64==(o+n-3)/64);
 if(!wp)memcpy(mem+o,p+2,n-2);
 writes++;busy=2;return APP_OK;
}
app_result_t port_i2c_probe(uint8_t a,uint32_t t){assert(a==0x50&&t);return never_ready||busy-->0?APP_IO_ERROR:APP_OK;}
app_result_t port_i2c_write_read(uint8_t a,const uint8_t *p,size_t pn,uint8_t *d,size_t n,uint32_t t){
 assert(a==0x50&&pn==2&&t);unsigned o=p[0]*256u+p[1];assert(o+n<=sizeof mem);memcpy(d,mem+o,n);return APP_OK;
}
int main(void){
 uint8_t input[130],out[130];for(unsigned i=0;i<130;i++)input[i]=(uint8_t)(i+1);
 assert(eeprom24_write(63,input,130,100)==APP_OK);assert(writes==4);
 assert(eeprom24_read(63,out,130,100)==APP_OK);assert(memcmp(input,out,130)==0);
 puts("PASS EEPROM split at 63/64/128/192, ACK polling, readback");
 assert(eeprom24_write(32767,input,2,100)==APP_INVALID_INPUT);
 assert(eeprom24_write(0,input,0,100)==APP_INVALID_INPUT);
 assert(eeprom24_read(0,NULL,1,100)==APP_INVALID_INPUT);
 assert(eeprom24_read(0,out,1,0)==APP_INVALID_INPUT);
 writes=0;assert(eeprom24_write(32767,input,1,20)==APP_OK&&writes==1);
 puts("PASS EEPROM boundary and invalid arguments");
 wp=1;assert(eeprom24_write(0,input,1,20)==APP_IO_ERROR);wp=0;
 fail=1;assert(eeprom24_write(0,input,1,20)==APP_IO_ERROR);fail=0;
 never_ready=1;tick=UINT32_MAX-2;assert(eeprom24_write(0,input,1,5)==APP_TIMEOUT);
 puts("PASS EEPROM write protection mismatch, bus failure, timeout across clock wrap");
}
