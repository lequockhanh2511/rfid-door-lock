#include "fake_eeprom.h"
#include "rfid_service.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
    test_eeprom_reset();config_store_t s={0},loaded;app_config_t c=test_config();
    assert(config_store_load(&s)==APP_NOT_READY&&!s.ready);
    for(int cut=0;cut<=161;cut++){
        test_eeprom_reset();test_write_budget=cut;
        (void)config_store_provision(&s,&c);test_write_budget=-1;
        assert(config_store_load(&s)==(cut<161?APP_NOT_READY:APP_OK));
    }
    test_eeprom_reset();
    assert(config_store_provision(&s,&c)==APP_OK&&s.revision==1);
    assert(config_store_provision(&s,&c)==APP_DENIED);
    unsigned char baseline[32768];memcpy(baseline,test_eeprom,sizeof baseline);
    app_config_t changed=c;memset(&changed.slots[1],0,sizeof changed.slots[1]);
    for(int cut=0;cut<=161;cut++){
        memcpy(test_eeprom,baseline,sizeof baseline);test_write_budget=-1;
        assert(config_store_load(&s)==APP_OK);test_write_budget=cut;
        app_result_t r=config_store_save(&s,&changed);
        assert((cut<161&&r==APP_IO_ERROR&&!s.ready)||(cut==161&&r==APP_OK));
        test_write_budget=-1;assert(config_store_load(&loaded)==APP_OK);
        assert(loaded.config.slots[1].used==(cut<161));
        assert(loaded.config.slots[2].used&&loaded.config.slots[2].uid.bytes[3]==2);
    }
    /* Repeat on a bank containing an older, still committed record. */
    memcpy(baseline,test_eeprom,sizeof baseline);
    for(int cut=0;cut<=161;cut++){
        memcpy(test_eeprom,baseline,sizeof baseline);test_write_budget=-1;
        assert(config_store_load(&s)==APP_OK);test_write_budget=cut;
        (void)config_store_save(&s,&c);test_write_budget=-1;
        assert(config_store_load(&loaded)==APP_OK);assert(loaded.config.slots[1].used==(cut==161));
    }
    /* Every single-bit corruption in a 160-byte newest record falls back. */
    unsigned char clean[CONFIG_RECORD_BYTES];memcpy(clean,test_eeprom,sizeof clean);
    for(unsigned bit=0;bit<CONFIG_RECORD_BYTES*8u;bit++){
        memcpy(test_eeprom,clean,sizeof clean);test_eeprom[bit/8]^=(unsigned char)(1u<<(bit%8));
        assert(config_store_load(&s)==APP_OK&&!s.config.slots[1].used);
    }
    memcpy(test_eeprom,clean,sizeof clean);
    test_eeprom[12]^=1; /* corrupt newest A -> previous B survives */
    assert(config_store_load(&s)==APP_OK&&!s.config.slots[1].used);
    test_read_fail=1;assert(config_store_load(&s)==APP_IO_ERROR&&!s.ready);test_read_fail=0;
    assert(config_store_load(&s)==APP_OK);
    app_command_t cmd={0};app_event_t e;cmd.request_id=7;cmd.kind=APP_CMD_ADD_CARD;
    cmd.data.uid=c.slots[1].uid;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_OK&&e.card_id==2&&e.used_count==10);
    rfid_service_command(&s,&cmd,&e);assert(e.result==APP_DUPLICATE);
    cmd.data.uid.bytes[2]=99;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_FULL);
    cmd.kind=APP_CMD_DELETE_CARD;cmd.data.card_id=2;rfid_service_command(&s,&cmd,&e);
    assert(e.result==APP_OK&&e.used_count==9&&e.occupied_mask==0x3fd);
    rfid_service_command(&s,&cmd,&e);assert(e.result==APP_NOT_FOUND);
    cmd.data.card_id=0;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_INVALID_INPUT);
    cmd.kind=APP_CMD_VERIFY_PIN;cmd.data.credential.role=APP_PIN_ADMIN;cmd.data.credential.pin=c.admin_pin;
    rfid_service_command(&s,&cmd,&e);assert(e.result==APP_OK);
    cmd.data.credential.pin.digits[0]='0';rfid_service_command(&s,&cmd,&e);assert(e.result==APP_DENIED);
    cmd.kind=APP_CMD_SET_PIN;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_OK);
    assert(config_store_load(&s)==APP_OK&&!memcmp(s.config.admin_pin.digits,"054321",7));
    test_write_budget=0;cmd.data.credential.pin=c.admin_pin;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_IO_ERROR&&!s.ready);
    cmd.kind=APP_CMD_VERIFY_PIN;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_NOT_READY);
    test_write_budget=-1;cmd.kind=APP_CMD_GET_STATUS;rfid_service_command(&s,&cmd,&e);assert(e.result==APP_OK&&s.ready);
    /* modulo sequence wrap chooses 0 as newer than UINT32_MAX */
    s.revision=UINT32_MAX-1;assert(config_store_save(&s,&changed)==APP_OK);assert(s.revision==UINT32_MAX);
    assert(config_store_save(&s,&c)==APP_OK&&s.revision==0);assert(config_store_load(&s)==APP_OK&&s.revision==0&&s.config.slots[1].used);
    /* Commit succeeded but final verification read failed: report uncertainty,
       then recover the actually committed new record on reload. */
    test_read_calls=0;test_read_fail_at=2;
    assert(config_store_save(&s,&changed)==APP_IO_ERROR&&!s.ready);
    test_read_fail_at=0;assert(config_store_load(&s)==APP_OK&&!s.config.slots[1].used);
    card_gate_t gate={0};assert(card_gate_update(&gate,APP_OK,true));
    for(int i=0;i<20;i++)assert(!card_gate_update(&gate,APP_OK,true));
    for(int i=0;i<20;i++)assert(!card_gate_update(&gate,APP_IO_ERROR,false));
    assert(!card_gate_update(&gate,APP_OK,true));
    for(int i=0;i<3;i++)assert(!card_gate_update(&gate,APP_NO_EVENT,false));
    assert(card_gate_update(&gate,APP_OK,true));
    puts("PASS config/service: 486 byte-cut trials, 1280 bit corruptions, uncertain commit readback, read/write faults, slot stability, full/duplicate, PIN, reboot, sequence wrap, held card");
}
