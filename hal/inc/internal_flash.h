#ifndef RFID_INTERNAL_FLASH_H
#define RFID_INTERNAL_FLASH_H
#include <stddef.h>
#include "app_types.h"
/* Each bank is a separate 1 KiB erase page. Offset and length must be even for programming. */
app_result_t flash_bank_read(uint8_t bank,size_t offset,uint8_t *data,size_t len);
app_result_t flash_bank_erase(uint8_t bank);
app_result_t flash_bank_program(uint8_t bank,size_t offset,const uint8_t *data,size_t len);
#endif
