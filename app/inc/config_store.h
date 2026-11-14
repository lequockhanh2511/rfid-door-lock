#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H
#include <stddef.h>
#include "app_types.h"
#define CONFIG_RECORD_BYTES 160u
#define CONFIG_BANK_STRIDE 256u
/* Wire format v1, little-endian; banks at 0x0000 and 0x0100.
 * Bytes 0..3 magic RF3!, 4 version, 5..7 reserved, 8..11 sequence,
 * 12..131 slots (used/length/UID[10]), 132..137 user PIN, 138..143 admin PIN,
 * 144..154 reserved, 155..158 CRC32 of bytes 0..154, 159 commit marker 0xA5.
 * Wire format is independent of C struct padding.
 */
typedef struct { app_card_slot_t slots[APP_CARD_CAPACITY]; app_pin_t user_pin,admin_pin; } app_config_t;
typedef struct { app_config_t config; uint32_t revision; uint8_t bank; bool ready; } config_store_t;
bool app_uid_valid(const app_uid_t *uid);
bool app_uid_equal(const app_uid_t *a,const app_uid_t *b);
bool app_pin_valid(const app_pin_t *pin);
void app_secure_clear(void *data,size_t size);
app_result_t config_store_load(config_store_t *store);
app_result_t config_store_save(config_store_t *store,const app_config_t *config);
/* Deliberate initial setup only: refuses a valid existing configuration. */
app_result_t config_store_provision(config_store_t *store,const app_config_t *config);
#endif
