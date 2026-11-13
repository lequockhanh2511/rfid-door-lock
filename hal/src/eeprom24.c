#include "eeprom24.h"
#include "driver_port.h"
#include <string.h>
static bool args(uint16_t o,const void *p,size_t n,uint32_t t) {
    return p && n && o<EEPROM24_BYTES && n<=EEPROM24_BYTES-o && driver_timeout_valid(t);
}
app_result_t eeprom24_read(uint16_t o,uint8_t *d,size_t n,uint32_t t) {
    if(!args(o,d,n,t)) return APP_INVALID_INPUT;
    uint8_t a[2]={(uint8_t)(o>>8),(uint8_t)o};
    return port_i2c_write_read(EEPROM24_ADDRESS,a,2,d,n,t);
}
app_result_t eeprom24_write(uint16_t o,const uint8_t *d,size_t n,uint32_t t) {
    if(!args(o,d,n,t)) return APP_INVALID_INPUT;
    uint32_t start=port_time_ms();
    while(n) {
        size_t chunk=EEPROM24_PAGE_BYTES-(o%EEPROM24_PAGE_BYTES);
        if(chunk>n) chunk=n;
        uint8_t packet[66],verify[64]; packet[0]=(uint8_t)(o>>8);packet[1]=(uint8_t)o;
        memcpy(packet+2,d,chunk);
        uint32_t left=driver_remaining(start,t); if(!left) return APP_TIMEOUT;
        app_result_t s=port_i2c_write(EEPROM24_ADDRESS,packet,chunk+2,left);
        if(s!=APP_OK) return s;
        for(;;) {
            left=driver_remaining(start,t); if(!left) return APP_TIMEOUT;
            s=port_i2c_probe(EEPROM24_ADDRESS,left<2u?left:2u);
            if(s==APP_OK) break;
            if(s==APP_NOT_READY||s==APP_INVALID_INPUT) return s;
            port_delay_ms(1); /* Bus mutex already released by probe. */
        }
        left=driver_remaining(start,t); if(!left) return APP_TIMEOUT;
        s=eeprom24_read(o,verify,chunk,left); if(s!=APP_OK) return s;
        if(memcmp(verify,d,chunk)!=0) return APP_IO_ERROR; /* e.g. WP high */
        o=(uint16_t)(o+chunk);d+=chunk;n-=chunk;
    }
    return APP_OK;
}
