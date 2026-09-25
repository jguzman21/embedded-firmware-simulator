#include "bootloader.h"

#include <stdio.h>

void bootloader_init(
    Bootloader *bootloader,
    const FirmwareImage *current_firmware,
    const FirmwareImage *staged_firmware)
{
    if (bootloader == NULL)
    {
        return;
    }

    bootloader->state = BOOT_STATE_RESET;
    bootloader->current_firmware = current_firmware;
    bootloader->staged_firmware = staged_firmware;
    bootloader->previous_firmware = NULL;
    bootloader->simulate_boot_failure = false;
    bootloader->application_ready = false;
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

        case BOOT_STATE_CHECK_FOR_UPDATE:
            return "CHECK_FOR_UPDATE";

        case BOOT_STATE_VALIDATE_UPDATE:
            return "VALIDATE_UPDATE";

        case BOOT_STATE_INSTALL_UPDATE:
            return "INSTALL_UPDATE";

        case BOOT_STATE_VERIFY_NEW_FIRMWARE:
            return "VERIFY_NEW_FIRMWARE";

        case BOOT_STATE_BOOT_APPLICATION:
            return "BOOT_APPLICATION";

        case BOOT_STATE_ROLLBACK:
            return "ROLLBACK";

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
        printf(
            "[BOOTLOADER] State: %s\n",
            bootloader_state_name(bootloader->state)
        );

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
                printf(
                    "[BOOTLOADER] Validating current firmware...\n"
                );

                if (firmware_is_valid(
                        bootloader->current_firmware))
                {
                    printf(
                        "[BOOTLOADER] Current firmware validation PASS\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_CHECK_FOR_UPDATE;
                }
                else
                {
                    printf(
                        "[BOOTLOADER] Current firmware validation FAIL\n"
                    );

                    bootloader->state = BOOT_STATE_RECOVERY;
                }

                break;

            case BOOT_STATE_CHECK_FOR_UPDATE:

                if (bootloader->staged_firmware == NULL)
                {
                    printf(
                        "[BOOTLOADER] No firmware update available\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_BOOT_APPLICATION;

                    break;
                }

                printf(
                    "[BOOTLOADER] Update available\n"
                );

                printf(
                    "[BOOTLOADER] Current version: %u\n",
                    bootloader->current_firmware->header.version
                );

                printf(
                    "[BOOTLOADER] Candidate version: %u\n\n",
                    bootloader->staged_firmware->header.version
                );

                if (bootloader->staged_firmware->header.version >
                    bootloader->current_firmware->header.version)
                {
                    bootloader->state =
                        BOOT_STATE_VALIDATE_UPDATE;
                }
                else
                {
                    printf(
                        "[BOOTLOADER] Candidate is not newer\n"
                    );

                    printf(
                        "[BOOTLOADER] Update rejected\n\n"
                    );

                    bootloader->staged_firmware = NULL;

                    bootloader->state =
                        BOOT_STATE_BOOT_APPLICATION;
                }

                break;

            case BOOT_STATE_VALIDATE_UPDATE:

                printf(
                    "[BOOTLOADER] Validating update...\n"
                );

                if (firmware_is_valid(
                        bootloader->staged_firmware))
                {
                    printf(
                        "[BOOTLOADER] Update validation PASS\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_INSTALL_UPDATE;
                }
                else
                {
                    printf(
                        "[BOOTLOADER] Update validation FAIL\n"
                    );

                    printf(
                        "[BOOTLOADER] Keeping current firmware\n\n"
                    );

                    bootloader->staged_firmware = NULL;

                    bootloader->state =
                        BOOT_STATE_BOOT_APPLICATION;
                }

                break;

            case BOOT_STATE_INSTALL_UPDATE:

                printf(
                    "[BOOTLOADER] Installing firmware update...\n"
                );

                bootloader->previous_firmware =
                    bootloader->current_firmware;

                bootloader->current_firmware =
                    bootloader->staged_firmware;

                bootloader->staged_firmware = NULL;

                printf(
                    "[BOOTLOADER] Update installation complete\n\n"
                );

                bootloader->state =
                    BOOT_STATE_VERIFY_NEW_FIRMWARE;

                break;

            case BOOT_STATE_VERIFY_NEW_FIRMWARE:

                printf(
                    "[BOOTLOADER] Verifying new firmware startup...\n"
                );

                if (bootloader->simulate_boot_failure)
                {
                    printf(
                        "[BOOTLOADER] Startup verification FAIL\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_ROLLBACK;
                }
                else
                {
                    printf(
                        "[BOOTLOADER] Startup verification PASS\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_BOOT_APPLICATION;
                }

                break;

            case BOOT_STATE_ROLLBACK:

                printf(
                    "[BOOTLOADER] Rolling back firmware...\n"
                );

                if (bootloader->previous_firmware != NULL)
                {
                    bootloader->current_firmware =
                        bootloader->previous_firmware;

                    bootloader->previous_firmware = NULL;

                    printf(
                        "[BOOTLOADER] Rollback complete\n"
                    );

                    printf(
                        "[BOOTLOADER] Previous firmware restored\n\n"
                    );

                    bootloader->state =
                        BOOT_STATE_BOOT_APPLICATION;
                }
                else
                {
                    printf(
                        "[BOOTLOADER] No previous firmware available\n"
                    );

                    bootloader->state =
                        BOOT_STATE_RECOVERY;
                }

                break;

            case BOOT_STATE_BOOT_APPLICATION:

                printf(
                    "[BOOTLOADER] Booting firmware version %u\n",
                    bootloader->current_firmware->header.version
                );

                printf(
                    "[BOOTLOADER] Application started\n\n"
                );

                bootloader->application_ready = true;
                bootloader->state = BOOT_STATE_COMPLETE;

                break;

            case BOOT_STATE_RECOVERY:

                printf(
                    "[BOOTLOADER] Entering recovery mode\n"
                );

                printf(
                    "[BOOTLOADER] System halted for recovery\n\n"
                );

                bootloader->application_ready = false;
                bootloader->state = BOOT_STATE_COMPLETE;

                break;

            case BOOT_STATE_COMPLETE:
                break;

            default:

                printf(
                    "[BOOTLOADER] Unknown state\n"
                );

                bootloader->state =
                    BOOT_STATE_RECOVERY;

                break;
        }
    }

    printf("[BOOTLOADER] Boot sequence complete\n");
}