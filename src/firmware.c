#include "firmware.h"
#include "crc32.h"

#include <stdio.h>

bool firmware_is_valid(const FirmwareImage *image)
{
    if (image == NULL)
    {
        printf("Firmware validation failed: NULL image\n");
        return false;
    }

    if (image->header.magic != FIRMWARE_MAGIC)
    {
        printf("Firmware validation failed: invalid magic\n");
        return false;
    }

    if (image->header.size == 0 ||
        image->header.size > MAX_FIRMWARE_SIZE)
    {
        printf("Firmware validation failed: invalid size\n");
        return false;
    }

    uint32_t calculated_crc =
        crc32_calculate(image->data, image->header.size);

    if (calculated_crc != image->header.crc32)
    {
        printf("Firmware validation failed: CRC mismatch\n");
        printf("  Expected: 0x%08X\n", image->header.crc32);
        printf("  Actual:   0x%08X\n", calculated_crc);
        return false;
    }

    return true;
}

void firmware_print_info(const FirmwareImage *image)
{
    if (image == NULL)
    {
        return;
    }

    printf("Firmware Information\n");
    printf("--------------------\n");
    printf("Magic:    0x%08X\n", image->header.magic);
    printf("Version:  %u\n", image->header.version);
    printf("Size:     %u bytes\n", image->header.size);
    printf("CRC32:    0x%08X\n", image->header.crc32);
}