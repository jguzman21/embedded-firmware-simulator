#include "firmware.h"

#include <stdio.h>

bool firmware_is_valid(const FirmwareImage *image)
{
    if (image == NULL)
    {
        return false;
    }

    if (image->header.magic != FIRMWARE_MAGIC)
    {
        return false;
    }

    if (image->header.size == 0 ||
        image->header.size > MAX_FIRMWARE_SIZE)
    {
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
    printf("Checksum: 0x%08X\n", image->header.checksum);
}