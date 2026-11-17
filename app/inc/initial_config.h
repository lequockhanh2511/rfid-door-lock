#ifndef INITIAL_CONFIG_H
#define INITIAL_CONFIG_H

#include "config_store.h"

/* Synthetic demo UIDs only. Slots 01/02 are enrolled from the reader in Admin. */
static const app_config_t app_initial_config = {
    .slots = {
        [2] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x01}}},
        [3] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x02}}},
        [4] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x03}}},
        [5] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x04}}},
        [6] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x05}}},
        [7] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x06}}},
        [8] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x07}}},
        [9] = {.used=true, .uid={.length=4, .bytes={0xfa,0xce,0x00,0x08}}},
    },
    .user_pin = {.digits=APP_DEMO_USER_PIN},
    .admin_pin = {.digits=APP_DEMO_ADMIN_PIN},
};
#endif
