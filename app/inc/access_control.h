#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H
#include "app_ipc.h"
/* Pure state machine: callback queues a copy; never waits for I/O. */
typedef app_result_t (*control_send_fn)(const app_command_t *cmd,void *ctx);
typedef struct {
    app_ui_snapshot_t view;
    bool admin,pending,cancelled,confirm_pin;
    bool input_overflow;
    uint8_t length;
    char input[APP_PIN_DIGITS+1u];
    app_pin_t current_pin,new_pin;
    uint8_t admin_failures,pin_stage;
    bool admin_locked;
    uint32_t admin_locked_at;
    app_pin_role_t pin_role,pending_role;
    app_command_kind_t pending_kind;
    uint32_t next_id,pending_id,pending_at,last_input,last_admin,mode_at,unlock_at;
    control_send_fn send;void *send_ctx;
} access_control_t;
void access_control_init(access_control_t *c,uint32_t now,control_send_fn send,void *ctx);
void access_control_tick(access_control_t *c,uint32_t now);
void access_control_key(access_control_t *c,char key,uint32_t now);
void access_control_event(access_control_t *c,const app_event_t *event,uint32_t now);
#endif
