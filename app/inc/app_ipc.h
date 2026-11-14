#ifndef RFID_APP_IPC_H
#define RFID_APP_IPC_H
#include "app_types.h"
typedef enum {
    APP_CMD_GET_STATUS, APP_CMD_LOOKUP_CARD, APP_CMD_ADD_CARD,
    APP_CMD_DELETE_CARD, APP_CMD_VERIFY_PIN, APP_CMD_SET_PIN
} app_command_kind_t;
typedef struct {
    uint32_t request_id; /* nonzero, no reuse while an old response may exist */
    app_command_kind_t kind;
    union {
        app_uid_t uid;
        uint8_t card_id;
        struct { app_pin_role_t role; app_pin_t pin; } credential;
    } data;
} app_command_t;
typedef enum {
    APP_EVENT_CARD_PRESENT, APP_EVENT_COMMAND_RESULT, APP_EVENT_STORAGE_FAULT
} app_event_kind_t;
typedef struct {
    app_event_kind_t kind;
    uint32_t request_id; /* zero for unsolicited events */
    app_command_kind_t command;
    app_result_t result;
    app_uid_t uid;
    uint8_t card_id;
    uint8_t used_count;
    uint16_t occupied_mask;
    uint32_t config_revision;
    uint32_t occurred_ms; /* CARD_PRESENT sample time; reject stale events. */
} app_event_t;
/* Implemented in app_ipc.c with FreeRTOS queues; no RTOS types leak here.
 * Copy messages by value. All pointer arguments must be non-NULL.
 * Task context only. timeout_ms=0 is nonblocking; no infinite timeout allowed.
 * init before any task. create failures return APP_NOT_READY and clean up.
 */
app_result_t app_ipc_init(void);
app_result_t app_ipc_send_command(const app_command_t *msg, uint32_t timeout_ms);
app_result_t app_ipc_receive_command(app_command_t *msg, uint32_t timeout_ms);
app_result_t app_ipc_send_event(const app_event_t *msg, uint32_t timeout_ms);
app_result_t app_ipc_receive_event(app_event_t *msg, uint32_t timeout_ms);
/* Latest-value mailbox: overwrite is permitted ONLY here. */
app_result_t app_ipc_publish_ui(const app_ui_snapshot_t *snapshot);
app_result_t app_ipc_receive_ui(app_ui_snapshot_t *snapshot, uint32_t timeout_ms);
#endif
