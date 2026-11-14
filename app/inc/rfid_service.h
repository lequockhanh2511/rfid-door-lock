#ifndef RFID_SERVICE_H
#define RFID_SERVICE_H
#include "app_ipc.h"
#include "config_store.h"
void rfid_service_command(config_store_t *store,const app_command_t *cmd,app_event_t *event);
/* Require three consecutive successful no-card polls before re-arming. Errors
 * never count as removal. Different UID while held also requires removal. */
typedef struct { bool held; uint8_t absent; } card_gate_t;
bool card_gate_update(card_gate_t *gate,app_result_t result,bool present);
#endif
