#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <stdint.h>
#include <stdbool.h>

#define FIRMWARE_MAGIC 0x46574D47
#define MAX_FIRMWARE_SIZE (64 * 1024)

typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t checksum;
} FirmwareHeader;

typedef struct
{
    FirmwareHeader header;
    uint8_t data[MAX_FIRMWARE_SIZE];
} FirmwareImage;

bool firmware_is_valid(const FirmwareImage *image);

void firmware_print_info(const FirmwareImage *image);

#endif