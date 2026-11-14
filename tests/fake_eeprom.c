#include "fake_eeprom.h"
#include "eeprom24.h"
#include <string.h>
unsigned char test_eeprom[32768];int test_write_budget=-1,test_read_fail,test_read_fail_at,test_read_calls;
void test_eeprom_reset(void){memset(test_eeprom,255,sizeof test_eeprom);test_write_budget=-1;test_read_fail=0;test_read_fail_at=0;test_read_calls=0;}
app_result_t eeprom24_read(uint16_t at,uint8_t *p,size_t n,uint32_t t){
    (void)t;test_read_calls++;if(test_read_fail||(test_read_fail_at&&test_read_calls==test_read_fail_at)||(size_t)at+n>sizeof test_eeprom)return APP_IO_ERROR;
    memcpy(p,test_eeprom+at,n);return APP_OK;
}
app_result_t eeprom24_write(uint16_t at,const uint8_t *p,size_t n,uint32_t t){
    (void)t;if((size_t)at+n>sizeof test_eeprom)return APP_IO_ERROR;
    for(size_t i=0;i<n;i++){
        if(test_write_budget==0)return APP_IO_ERROR;
        if(test_write_budget>0)test_write_budget--;
        test_eeprom[at+i]=p[i];
    }return APP_OK;
}
app_config_t test_config(void){
    app_config_t c={0};memcpy(c.user_pin.digits,APP_DEMO_USER_PIN,7);memcpy(c.admin_pin.digits,APP_DEMO_ADMIN_PIN,7);
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++){
        c.slots[i].used=true;c.slots[i].uid.length=4;c.slots[i].uid.bytes[0]=0xf0;
        c.slots[i].uid.bytes[3]=(uint8_t)i; /* TEST_ONLY, no physical cards claimed. */
    }return c;
}
