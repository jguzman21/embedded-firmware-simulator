#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdbool.h>

#include "firmware.h"

typedef enum
{
    BOOT_STATE_RESET,
    BOOT_STATE_SELF_TEST,
    BOOT_STATE_VALIDATE_FIRMWARE,
    BOOT_STATE_CHECK_FOR_UPDATE,
    BOOT_STATE_VALIDATE_UPDATE,
    BOOT_STATE_INSTALL_UPDATE,
    BOOT_STATE_VERIFY_NEW_FIRMWARE,
    BOOT_STATE_BOOT_APPLICATION,
    BOOT_STATE_ROLLBACK,
    BOOT_STATE_RECOVERY,
    BOOT_STATE_COMPLETE
} BootState;

typedef struct
{
    BootState state;

    const FirmwareImage *current_firmware;
    const FirmwareImage *staged_firmware;
    const FirmwareImage *previous_firmware;

    bool simulate_boot_failure;
} Bootloader;

void bootloader_init(
    Bootloader *bootloader,
    const FirmwareImage *current_firmware,
    const FirmwareImage *staged_firmware
);

void bootloader_run(Bootloader *bootloader);

const char *bootloader_state_name(BootState state);

#endif