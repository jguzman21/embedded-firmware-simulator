#include <stdio.h>

#include "firmware.h"

int main(void)
{
    printf("Embedded Firmware Simulator\n");
    printf("===========================\n\n");

    FirmwareImage firmware = {0};

    firmware.header.magic = FIRMWARE_MAGIC;
    firmware.header.version = 1;
    firmware.header.size = 4096;
    firmware.header.checksum = 0x12345678;

    firmware_print_info(&firmware);

    printf("\nValidation: ");

    if (firmware_is_valid(&firmware))
    {
        printf("PASS\n");
    }
    else
    {
        printf("FAIL\n");
    }

    return 0;
}