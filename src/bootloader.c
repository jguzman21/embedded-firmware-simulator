#include "bootloader.h"

#include <stdio.h>

void bootloader_init(Bootloader *bootloader,
                     const FirmwareImage *firmware)
{
    if (bootloader == NULL)
    {
        return;
    }

    bootloader->state = BOOT_STATE_RESET;
    bootloader->firmware = firmware;
}

const char *bootloader_state_name(BootState state)
{
    switch (state)
    {
        case BOOT_STATE_RESET:
            return "RESET";

        case BOOT_STATE_SELF_TEST:
            return "SELF_TEST";

        case BOOT_STATE_VALIDATE_FIRMWARE:
            return "VALIDATE_FIRMWARE";

        case BOOT_STATE_BOOT_APPLICATION:
            return "BOOT_APPLICATION";

        case BOOT_STATE_RECOVERY:
            return "RECOVERY";

        case BOOT_STATE_COMPLETE:
            return "COMPLETE";

        default:
            return "UNKNOWN";
    }
}

void bootloader_run(Bootloader *bootloader)
{
    if (bootloader == NULL)
    {
        return;
    }

    while (bootloader->state != BOOT_STATE_COMPLETE)
    {
        printf("[BOOTLOADER] State: %s\n",
               bootloader_state_name(bootloader->state));

        switch (bootloader->state)
        {
            case BOOT_STATE_RESET:
                bootloader->state = BOOT_STATE_SELF_TEST;
                break;

            case BOOT_STATE_SELF_TEST:
                printf("[BOOTLOADER] Running self-test...\n");
                printf("[BOOTLOADER] Self-test PASS\n\n");

                bootloader->state = BOOT_STATE_VALIDATE_FIRMWARE;
                break;

            case BOOT_STATE_VALIDATE_FIRMWARE:
                printf("[BOOTLOADER] Validating firmware...\n");

                if (firmware_is_valid(bootloader->firmware))
                {
                    printf("[BOOTLOADER] Firmware validation PASS\n\n");
                    bootloader->state = BOOT_STATE_BOOT_APPLICATION;
                }
                else
                {
                    printf("[BOOTLOADER] Firmware validation FAIL\n\n");
                    bootloader->state = BOOT_STATE_RECOVERY;
                }

                break;

            case BOOT_STATE_BOOT_APPLICATION:
                printf("[BOOTLOADER] Firmware accepted\n");
                printf("[BOOTLOADER] Booting application...\n\n");

                bootloader->state = BOOT_STATE_COMPLETE;
                break;

            case BOOT_STATE_RECOVERY:
                printf("[BOOTLOADER] Entering recovery mode\n");
                printf("[BOOTLOADER] System halted for recovery\n\n");

                bootloader->state = BOOT_STATE_COMPLETE;
                break;

            case BOOT_STATE_COMPLETE:
                break;

            default:
                printf("[BOOTLOADER] Unknown state\n");
                bootloader->state = BOOT_STATE_RECOVERY;
                break;
        }
    }

    printf("[BOOTLOADER] Boot sequence complete\n");
}