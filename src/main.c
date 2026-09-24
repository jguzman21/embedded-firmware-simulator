#include <stdio.h>
#include <string.h>

#include "bootloader.h"
#include "crc32.h"
#include "firmware.h"

int main(void)
{
    printf("=================================\n");
    printf(" Embedded Firmware Simulator\n");
    printf("=================================\n\n");

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

    printf("\n=================================\n");
    printf(" Starting Bootloader\n");
    printf("=================================\n\n");

    Bootloader bootloader;

    bootloader_init(&bootloader, &firmware);
    bootloader_run(&bootloader);

    return 0;
}