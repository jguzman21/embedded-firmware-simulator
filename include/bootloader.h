#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdbool.h>

#include "firmware.h"

typedef enum
{
    BOOT_STATE_RESET,
    BOOT_STATE_SELF_TEST,
    BOOT_STATE_VALIDATE_FIRMWARE,
    BOOT_STATE_BOOT_APPLICATION,
    BOOT_STATE_RECOVERY,
    BOOT_STATE_COMPLETE
} BootState;

typedef struct
{
    BootState state;
    const FirmwareImage *firmware;
} Bootloader;

void bootloader_init(Bootloader *bootloader,
                     const FirmwareImage *firmware);

void bootloader_run(Bootloader *bootloader);

const char *bootloader_state_name(BootState state);

#endif