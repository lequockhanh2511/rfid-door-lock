#ifndef RFID_APP_TYPES_H
#define RFID_APP_TYPES_H
#include <stdbool.h>
#include <stdint.h>
#include "app_cfg.h"
typedef enum {
    APP_OK = 0, APP_DUPLICATE, APP_FULL, APP_NOT_FOUND,
    APP_INVALID_INPUT, APP_IO_ERROR, APP_BUSY, APP_TIMEOUT,
    APP_DENIED, APP_NOT_READY, APP_NO_EVENT
} app_result_t;
typedef struct { uint8_t length; uint8_t bytes[APP_UID_MAX_BYTES]; } app_uid_t;
typedef struct { bool used; app_uid_t uid; } app_card_slot_t;
typedef enum { APP_PIN_USER, APP_PIN_ADMIN } app_pin_role_t;
typedef struct { char digits[APP_PIN_DIGITS + 1u]; } app_pin_t;
typedef enum {
    APP_STATE_BOOT, APP_STATE_SETUP_REQUIRED, APP_STATE_IDLE,
    APP_STATE_PIN_ENTRY, APP_STATE_UNLOCKED, APP_STATE_ADMIN_AUTH,
    APP_STATE_ADMIN_MENU, APP_STATE_ADD_WAIT_CARD,
    APP_STATE_DELETE_INPUT, APP_STATE_DELETE_CONFIRM,
    APP_STATE_LIST_CARDS, APP_STATE_CHANGE_PIN,
    APP_STATE_WAIT_RESULT, APP_STATE_ERROR
} app_state_t;
/* IDs 1..10; zero means 'no slot'. UID equality includes length. */
typedef struct {
    app_state_t state;
    app_result_t result;
    bool unlocked;
    uint8_t card_id;
    uint8_t used_count;
    uint8_t entered_digits; /* Never put PIN text into UI messages. */
    uint16_t occupied_mask; /* bit 0 = slot 01, ... bit 9 = slot 10 */
    uint32_t remaining_ms;
    uint8_t selected_id; /* list selection or typed deletion number */
    bool confirm_pin;
    app_pin_role_t pin_role;
} app_ui_snapshot_t;
#endif
