#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

uint32_t crc32_calculate(const uint8_t *data, size_t length);

#endif