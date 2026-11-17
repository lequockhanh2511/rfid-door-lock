#ifndef FAKE_FLASH_H
#define FAKE_FLASH_H
#include "config_store.h"
extern unsigned char test_flash[2 * CONFIG_BANK_STRIDE];
extern int test_write_budget,test_read_fail,test_read_fail_at,test_read_calls;
void test_flash_reset(void);
app_config_t test_config(void);
#endif
