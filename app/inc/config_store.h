#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H
#include <stddef.h>
#include "app_types.h"
#define CONFIG_RECORD_BYTES 160u
#define CONFIG_BANK_STRIDE 1024u
/* Wire format v1, little-endian, independent of C structure padding.
 * 0..3 RF3!, 4 version, 5..7 reserved, 8..11 sequence,
 * 12..131 fixed slots (used, length, UID[10]), 132..137 user PIN,
 * 138..143 admin PIN, 144..154 reserved, 155..158 CRC32,
 * 159 commit marker 0xA5. Commit the final halfword last.
 */
typedef struct { app_card_slot_t slots[APP_CARD_CAPACITY]; app_pin_t user_pin,admin_pin; } app_config_t;
typedef struct { app_config_t config; uint32_t revision; uint8_t bank; bool ready; } config_store_t;
bool app_uid_valid(const app_uid_t *uid);
bool app_uid_equal(const app_uid_t *a,const app_uid_t *b);
bool app_pin_valid(const app_pin_t *pin);
void app_secure_clear(void *data,size_t size);
app_result_t config_store_load(config_store_t *store);
/* Load existing data, or seed eight virtual cards only on entirely erased pages. */
app_result_t config_store_initialize(config_store_t *store);
app_result_t config_store_save(config_store_t *store,const app_config_t *config);
/* Initial setup only: refuses existing valid or corrupt configuration. */
app_result_t config_store_provision(config_store_t *store,const app_config_t *config);
#endif
