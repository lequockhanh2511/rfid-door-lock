#ifndef DRIVER_PORT_H
#define DRIVER_PORT_H
#include "hal_ports.h"
void port_rc522_reset(bool high);
void port_keypad_row(uint8_t row, bool active);
uint8_t port_keypad_columns(void); /* bit set = low/pressed */
void port_set_lock_leds(bool unlocked);
void port_delay_ms(uint32_t ms);
void port_delay_us(uint32_t us);
app_result_t port_i2c_probe(uint8_t address, uint32_t timeout_ms);
/* All blocking driver APIs require 1..INT32_MAX ms. GPIO calls are bounded. */
static inline bool driver_timeout_valid(uint32_t ms) { return ms > 0u && ms <= INT32_MAX; }
static inline uint32_t driver_remaining(uint32_t start, uint32_t budget) {
    uint32_t elapsed = port_time_ms() - start;
    return elapsed < budget ? budget - elapsed : 0u;
}
#endif
