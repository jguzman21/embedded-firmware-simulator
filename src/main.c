#include <stdio.h>
#include <string.h>

#include "firmware.h"
#include "crc32.h"

int main(void)
{
    printf("Embedded Firmware Simulator\n");
    printf("===========================\n\n");

    FirmwareImage firmware = {0};

    firmware.header.magic = FIRMWARE_MAGIC;
    firmware.header.version = 1;

    const char *firmware_data =
        "Embedded firmware version 1.0";

    firmware.header.size = strlen(firmware_data);

    memcpy(
        firmware.data,
        firmware_data,
        firmware.header.size
    );

    firmware.header.crc32 =
        crc32_calculate(
            firmware.data,
            firmware.header.size
        );
    
    
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