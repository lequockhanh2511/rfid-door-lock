#ifndef RFID_APP_TASKS_H
#define RFID_APP_TASKS_H
#include "app_types.h"
/* Call once after BSP and IPC setup, before scheduler. Not implemented in M1. */
app_result_t task_control_create(void);
app_result_t task_rfid_create(void);
app_result_t task_ui_create(void);
#endif
