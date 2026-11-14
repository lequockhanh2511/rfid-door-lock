#ifndef FAKE_EEPROM_H
#define FAKE_EEPROM_H
#include "config_store.h"
extern unsigned char test_eeprom[32768];
extern int test_write_budget,test_read_fail;
void test_eeprom_reset(void);
app_config_t test_config(void);
#endif
