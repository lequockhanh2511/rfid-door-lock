#ifndef EEPROM24_H
#define EEPROM24_H
#include <stddef.h>
#include "app_types.h"
#define EEPROM24_BYTES 32768u
#define EEPROM24_PAGE_BYTES 64u
#define EEPROM24_ADDRESS 0x50u
app_result_t eeprom24_read(uint16_t offset,uint8_t *data,size_t length,uint32_t timeout_ms);
/* Splits pages, ACK polls, then verifies each page. Not an atomic record write. */
app_result_t eeprom24_write(uint16_t offset,const uint8_t *data,size_t length,uint32_t timeout_ms);
#endif
