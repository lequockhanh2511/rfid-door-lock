#ifndef RFID_APP_CFG_H
#define RFID_APP_CFG_H
/* Design defaults; timing/stack/queue sizing still require board validation. */
#define APP_CARD_CAPACITY 10u
#define APP_UID_MAX_BYTES 10u
#define APP_PIN_DIGITS 6u
#define APP_UNLOCK_MS 5000u
#define APP_INPUT_TIMEOUT_MS 20000u
#define APP_ADMIN_TIMEOUT_MS 60000u
#define APP_KEYPAD_SCAN_MS 10u
#define APP_KEYPAD_DEBOUNCE_MS 30u
#define APP_RFID_POLL_MS 50u
#define APP_UI_PERIOD_MS 100u
#define APP_COMMAND_QUEUE_DEPTH 8u
#define APP_EVENT_QUEUE_DEPTH 16u
#define APP_UI_QUEUE_DEPTH 1u
#define APP_CONTROL_PRIORITY 3u
#define APP_RFID_PRIORITY 2u
#define APP_UI_PRIORITY 1u
/* FreeRTOS stack sizes below are words, NOT bytes; provisional. */
#define APP_CONTROL_STACK_WORDS 512u
#define APP_RFID_STACK_WORDS 768u
#define APP_UI_STACK_WORDS 512u
/* Provisioning only. Never restore these on an ordinary failed EEPROM read. */
#define APP_DEMO_USER_PIN "123456"
#define APP_DEMO_ADMIN_PIN "654321"
#endif
