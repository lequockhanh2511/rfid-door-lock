#include "device_api.h"
#include "driver_port.h"
#include "rc522_protocol.h"
#include <string.h>
/* MFRC522 datasheet Rev3.9; one SPI owner. Multi-card collisions are rejected. */
enum { CMD=0x01, IRQ=0x04, ERROR=0x06, FIFO=0x09, LEVEL=0x0a,
       CONTROL=0x0c, FRAMING=0x0d, MODE=0x11, TXMODE=0x12,
       RXMODE=0x13, TXCONTROL=0x14, TXASK=0x15, TMODE=0x2a,
       TPRESCALER=0x2b, TRELOADH=0x2c, TRELOADL=0x2d, VERSION=0x37 };
typedef struct { uint32_t start,budget; } deadline_t;
static bool ready;
uint16_t rc522_crc_a(const uint8_t *d,size_t n) {
    uint16_t crc=0x6363;
    while(n--) { uint8_t x=(uint8_t)(*d++ ^ crc);x^=(uint8_t)(x<<4);
        crc=(uint16_t)((crc>>8)^((uint16_t)x<<8)^((uint16_t)x<<3)^(x>>4)); }
    return crc;
}
static app_result_t reg_io(uint8_t reg,uint8_t *value,bool read,deadline_t *d) {
    uint8_t tx[2]={(uint8_t)((reg<<1)|(read?0x80:0)),read?0:*value},rx[2];
    uint32_t left=driver_remaining(d->start,d->budget); if(!left) return APP_TIMEOUT;
    app_result_t s=port_spi_transfer(tx,rx,2,left);if(s==APP_OK&&read)*value=rx[1];return s;
}
static app_result_t wr(uint8_t r,uint8_t v,deadline_t *d) {return reg_io(r,&v,false,d);}
static app_result_t rd(uint8_t r,uint8_t *v,deadline_t *d) {return reg_io(r,v,true,d);}
#define TRY(x) do { app_result_t s_=(x); if(s_!=APP_OK) return s_; } while(0)
static app_result_t exchange(const uint8_t *tx,size_t n,uint8_t lastbits,
                            uint8_t *rx,size_t cap,size_t *got,deadline_t *d) {
    *got=0;
    TRY(wr(CMD,0,d)); TRY(wr(IRQ,0x7f,d)); TRY(wr(LEVEL,0x80,d));
    for(size_t i=0;i<n;i++) TRY(wr(FIFO,tx[i],d));
    TRY(wr(FRAMING,lastbits,d)); TRY(wr(CMD,0x0c,d)); TRY(wr(FRAMING,(uint8_t)(0x80|lastbits),d));
    for(;;) {
        uint8_t irq;TRY(rd(IRQ,&irq,d));
        if(irq&0x30u) break;
        if(irq&1u) return APP_NO_EVENT; /* RF hardware timer, not SPI failure. */
        if(!driver_remaining(d->start,d->budget)) return APP_TIMEOUT;
        port_delay_ms(1);
    }
    uint8_t error,count,bits;TRY(rd(ERROR,&error,d));
    if(error&0x1bu) return APP_IO_ERROR; /* collision/parity/protocol/overflow */
    TRY(rd(LEVEL,&count,d));TRY(rd(CONTROL,&bits,d));
    if(count>cap||(bits&7u)!=0u) return APP_IO_ERROR;
    for(uint8_t i=0;i<count;i++) TRY(rd(FIFO,&rx[i],d));
    *got=count;return APP_OK;
}
app_result_t rc522_init(void) {
    ready=false;port_rc522_reset(false);port_delay_ms(2);port_rc522_reset(true);port_delay_ms(50);
    deadline_t d={port_time_ms(),100};uint8_t version;
    TRY(rd(VERSION,&version,&d));
    if(version!=0x91u&&version!=0x92u) return APP_NOT_READY;
    TRY(wr(TMODE,0x80,&d));TRY(wr(TPRESCALER,0xa9,&d));
    TRY(wr(TRELOADH,0x03,&d));TRY(wr(TRELOADL,0xe8,&d)); /* ~25ms RF timeout */
    TRY(wr(TXMODE,0,&d));TRY(wr(RXMODE,0,&d));
    TRY(wr(TXASK,0x40,&d));TRY(wr(MODE,0x3d,&d));
    uint8_t antenna;TRY(rd(TXCONTROL,&antenna,&d));TRY(wr(TXCONTROL,(uint8_t)(antenna|3u),&d));
    port_delay_ms(5);ready=true;return APP_OK;
}
app_result_t rc522_poll(app_uid_t *uid,bool *present,uint32_t timeout_ms) {
    if(!uid||!present||!driver_timeout_valid(timeout_ms))return APP_INVALID_INPUT;
    memset(uid,0,sizeof(*uid));*present=false;if(!ready)return APP_NOT_READY;
    deadline_t d={port_time_ms(),timeout_ms};uint8_t rx[10];size_t n;
    uint8_t wake=0x52; /* WUPA: cards were HALTed after preceding successful poll. */
    app_result_t s=exchange(&wake,1,7,rx,sizeof rx,&n,&d);
    if(s!=APP_OK)return s;
    if(n!=2)return APP_IO_ERROR;
    app_uid_t candidate={0};
    for(unsigned level=0;level<3;level++) {
        uint8_t frame[9]={(uint8_t)(0x93u+level*2u),0x20};
        TRY(exchange(frame,2,0,rx,sizeof rx,&n,&d));
        if(n!=5||(uint8_t)(rx[0]^rx[1]^rx[2]^rx[3])!=rx[4])return APP_IO_ERROR;
        bool cascade=rx[0]==0x88;
        if(cascade&&level==2)return APP_IO_ERROR;
        unsigned bytes=cascade?3u:4u;
        memcpy(candidate.bytes+candidate.length,rx+(cascade?1:0),bytes);
        candidate.length=(uint8_t)(candidate.length+bytes);
        frame[1]=0x70;memcpy(frame+2,rx,5);
        uint16_t crc=rc522_crc_a(frame,7);frame[7]=(uint8_t)crc;frame[8]=(uint8_t)(crc>>8);
        TRY(exchange(frame,9,0,rx,sizeof rx,&n,&d));
        if(n!=3)return APP_IO_ERROR;
        crc=rc522_crc_a(rx,1);
        if(rx[1]!=(uint8_t)crc||rx[2]!=(uint8_t)(crc>>8))return APP_IO_ERROR;
        if(((rx[0]&4u)!=0u)!=cascade)return APP_IO_ERROR;
        if(!cascade) {
            uint8_t halt[4]={0x50,0};crc=rc522_crc_a(halt,2);
            halt[2]=(uint8_t)crc;halt[3]=(uint8_t)(crc>>8);
            s=exchange(halt,4,0,rx,sizeof rx,&n,&d);
            /* A compliant HALT has no answer. Any transport error is propagated. */
            if(s!=APP_NO_EVENT)return s==APP_OK?APP_IO_ERROR:s;
            *uid=candidate;*present=true;return APP_OK;
        }
    }
    return APP_IO_ERROR;
}
