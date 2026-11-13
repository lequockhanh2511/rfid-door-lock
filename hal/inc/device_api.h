#ifndef RFID_DEVICE_API_H
#define RFID_DEVICE_API_H
#include "app_types.h"
/* Nguyen Huu Anh: raw card presence. NO_EVENT is distinct from IO_ERROR. */
app_result_t rc522_init(void);
/* Returns APP_NO_EVENT with present=false on RF timer expiry; transport failures
 * are distinct. Repeated successful polls may return the same card; task filters it. */
app_result_t rc522_poll(app_uid_t *uid, bool *present, uint32_t timeout_ms);
/* Ha Xuan Khai: raw 16-bit matrix; debounce belongs to keypad logic.
 * Key order specified by keypad driver config after actual wiring is known.
 */
app_result_t keypad_read_matrix(uint16_t *pressed_mask);
/* Le Quoc Khanh: hardware output separated from OLED I2C delays.
 * lock_output_init_closed runs at boot before scheduler.
 * set is a bounded GPIO-only operation, sole caller = control task.
 */
void lock_output_init_closed(void);
void lock_output_set(bool unlocked);
app_result_t oled_init(uint32_t timeout_ms);
app_result_t oled_present(const app_ui_snapshot_t *view, uint32_t timeout_ms);
#endif
