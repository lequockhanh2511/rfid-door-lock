/* Host-only queue model ABI. NEVER on firmware include path. */
#ifndef TEST_FREERTOS_H
#define TEST_FREERTOS_H
#include <stddef.h>
#include <stdint.h>
typedef uint32_t TickType_t;
#define configTICK_RATE_HZ 1000u
#define portMAX_DELAY UINT32_MAX
#define pdPASS 1
#endif
