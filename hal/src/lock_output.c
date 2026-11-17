#include "device_api.h"
#include "driver_port.h"
void lock_output_init_closed(void) {port_set_lock_servo(false);}
void lock_output_set(bool unlocked) {port_set_lock_servo(unlocked);}
