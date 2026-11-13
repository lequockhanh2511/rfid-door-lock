#ifndef RFID_HAL_PORTS_H
#define RFID_HAL_PORTS_H
#include <stddef.h>
#include <stdint.h>
#include "app_types.h"
/* Adapter interfaces, NOT replacements for STM32 HAL. Task context only.
 * Buffers remain owned by caller; calls complete before returning.
 * Require finite timeout, valid buffers and lengths. No hidden infinite waits.
 */
app_result_t port_spi_transfer(const uint8_t *tx, uint8_t *rx,
                               size_t length, uint32_t timeout_ms);
/* 7-bit I2C address; adapter handles STM32 HAL address encoding.
 * Each call obtains/releases the shared I2C mutex within the timeout budget.
 * write_read uses a repeated start and keeps ownership for the whole transfer.
 */
app_result_t port_i2c_write(uint8_t address, const uint8_t *data,
                           size_t length, uint32_t timeout_ms);
app_result_t port_i2c_write_read(uint8_t address, const uint8_t *prefix,
                                size_t prefix_length, uint8_t *data,
                                size_t length, uint32_t timeout_ms);
uint32_t port_time_ms(void); /* monotonic uint32_t, wraps; use elapsed subtraction */
#endif
