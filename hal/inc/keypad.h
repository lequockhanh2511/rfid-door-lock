#ifndef KEYPAD_H
#define KEYPAD_H
#include "app_types.h"
typedef struct {
    uint16_t candidate;
    uint32_t changed_at;
    bool latched,blocked;
} keypad_state_t;
void keypad_state_init(keypad_state_t *state,uint32_t now);
/* Pure logic: one event per press, no repeats; chord blocks until stable release. */
app_result_t keypad_update(keypad_state_t *state,uint16_t mask,uint32_t now,char *key);
#endif
