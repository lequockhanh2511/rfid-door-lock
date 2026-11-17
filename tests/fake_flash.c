#include "fake_flash.h"
#include "internal_flash.h"
#include <string.h>
unsigned char test_flash[2 * CONFIG_BANK_STRIDE];int test_write_budget=-1,test_read_fail,test_read_fail_at,test_read_calls;
void test_flash_reset(void){memset(test_flash,255,sizeof test_flash);test_write_budget=-1;test_read_fail=0;test_read_fail_at=0;test_read_calls=0;}
app_result_t flash_bank_read(uint8_t bank,size_t off,uint8_t *p,size_t n){
    test_read_calls++;if(bank>=2||off>CONFIG_BANK_STRIDE||n>CONFIG_BANK_STRIDE-off||test_read_fail||(test_read_fail_at&&test_read_calls==test_read_fail_at))return APP_IO_ERROR;
    memcpy(p,test_flash+bank*CONFIG_BANK_STRIDE+off,n);return APP_OK;
}
app_result_t flash_bank_erase(uint8_t bank){
    if(bank>=2||test_write_budget==0)return APP_IO_ERROR;
    if(test_write_budget>0)test_write_budget--;
    memset(test_flash+bank*CONFIG_BANK_STRIDE,255,CONFIG_BANK_STRIDE);return APP_OK;
}
app_result_t flash_bank_program(uint8_t bank,size_t off,const uint8_t *p,size_t n){
    if(bank>=2||off>CONFIG_BANK_STRIDE||n>CONFIG_BANK_STRIDE-off||((off|n)&1u))return APP_IO_ERROR;
    for(size_t i=0;i<n;i++){
        if(test_write_budget==0)return APP_IO_ERROR;
        if(test_write_budget>0)test_write_budget--;
        size_t at=bank*CONFIG_BANK_STRIDE+off+i;
        if(test_flash[at]!=255)return APP_IO_ERROR;
        test_flash[at]=p[i];
    }return APP_OK;
}
app_config_t test_config(void){
    app_config_t c={0};memcpy(c.user_pin.digits,APP_DEMO_USER_PIN,7);memcpy(c.admin_pin.digits,APP_DEMO_ADMIN_PIN,7);
    for(unsigned i=0;i<APP_CARD_CAPACITY;i++){
        c.slots[i].used=true;c.slots[i].uid.length=4;c.slots[i].uid.bytes[0]=0xf0;
        c.slots[i].uid.bytes[3]=(uint8_t)i; /* TEST_ONLY, no physical cards claimed. */
    }return c;
}
