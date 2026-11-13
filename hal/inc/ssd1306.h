#ifndef SSD1306_H
#define SSD1306_H
#include "app_types.h"
#ifndef SSD1306_ADDRESS
#define SSD1306_ADDRESS 0x3cu
#endif
#define SSD1306_FRAME_BYTES 1024u
void ssd1306_clear(void);
/* 8 text rows, 21 columns; out-of-range input rejected; clipped at right edge. */
app_result_t ssd1306_text(uint8_t row,uint8_t col,const char *text);
const uint8_t *ssd1306_framebuffer(void);
app_result_t ssd1306_flush(uint32_t timeout_ms);
#endif
