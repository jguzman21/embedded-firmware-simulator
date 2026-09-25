#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "bootloader.h"
#include "crc32.h"
#include "firmware.h"
#include "application.h"

static FirmwareImage create_firmware(
    uint32_t version,
    const char *data)
{
    FirmwareImage firmware = {0};

    firmware.header.magic = FIRMWARE_MAGIC;
    firmware.header.version = version;
    firmware.header.size = strlen(data);

    memcpy(
        firmware.data,
        data,
        firmware.header.size
    );

    firmware.header.crc32 =
        crc32_calculate(
            firmware.data,
            firmware.header.size
        );
    

    return firmware;
}

int main(int argc, char *argv[])
{
    bool simulate_imu_hang = false;
    bool has_rebooted = false;

    if (argc > 1 &&
        strcmp(argv[1], "--hang-imu") == 0)
    {
    simulate_imu_hang = true;
    }   

    printf("=================================\n");
    printf(" Embedded Firmware Simulator\n");
    printf("=================================\n\n");

    FirmwareImage firmware_v1 =
        create_firmware(
            1,
            "Embedded firmware version 1.0"
        );

    FirmwareImage firmware_v2 =
        create_firmware(
            2,
            "Embedded firmware version 2.0"
        );

    

    printf("Current firmware:\n");
    firmware_print_info(&firmware_v1);

    printf("\nAvailable update:\n");
    firmware_print_info(&firmware_v2);

    printf("\n=================================\n");
    printf(" Starting Bootloader\n");
    printf("=================================\n\n");

    bool system_running = true;

    while (system_running)
    {
        Bootloader bootloader;

        bootloader_init(
            &bootloader,
            &firmware_v1,
            &firmware_v2
        );

        bootloader.simulate_boot_failure = false;

        bootloader_run(&bootloader);

        if (!bootloader.application_ready)
        {
            printf(
                "[SYSTEM] Application was not started.\n"
            );

            break;
        }

    
        bool inject_fault =
            simulate_imu_hang && !has_rebooted;

        ApplicationResult result =
            application_run(
            5000,
            inject_fault
        );

        if (result == APPLICATION_RESULT_WATCHDOG_FAULT)
        {
            printf("\n");
            printf("=================================\n");
            printf(" SYSTEM RESET REQUESTED\n");
            printf("=================================\n\n");

            has_rebooted = true;

            printf(
                "[SYSTEM] Restarting bootloader...\n\n"
            );

            continue;
        }

        system_running = false;
    }

    return 0;
}