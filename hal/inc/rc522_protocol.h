#ifndef RC522_PROTOCOL_H
#define RC522_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
/* ISO14443A CRC_A, initial 0x6363; little endian on air. */
uint16_t rc522_crc_a(const uint8_t *data,size_t length);
#endif
