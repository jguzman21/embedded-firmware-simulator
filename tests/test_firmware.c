#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "crc32.h"
#include "firmware.h"

static FirmwareImage create_test_firmware(void)
{
    FirmwareImage firmware = {0};

    const char *data = "test firmware";

    firmware.header.magic = FIRMWARE_MAGIC;
    firmware.header.version = 1;
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

static void test_valid_firmware(void)
{
    FirmwareImage firmware =
        create_test_firmware();

    assert(firmware_is_valid(&firmware));
}

static void test_invalid_magic(void)
{
    FirmwareImage firmware =
        create_test_firmware();

    firmware.header.magic = 0xDEADBEEF;

    assert(!firmware_is_valid(&firmware));
}

static void test_invalid_size(void)
{
    FirmwareImage firmware =
        create_test_firmware();

    firmware.header.size =
        MAX_FIRMWARE_SIZE + 1;

    assert(!firmware_is_valid(&firmware));
}

static void test_corrupted_firmware(void)
{
    FirmwareImage firmware =
        create_test_firmware();

    firmware.data[0] ^= 0xFF;

    assert(!firmware_is_valid(&firmware));
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(
            stderr,
            "Usage: %s <test_name>\n",
            argv[0]
        );

        return 1;
    }

    if (strcmp(argv[1], "valid") == 0)
    {
        test_valid_firmware();
    }
    else if (strcmp(argv[1], "invalid_magic") == 0)
    {
        test_invalid_magic();
    }
    else if (strcmp(argv[1], "invalid_size") == 0)
    {
        test_invalid_size();
    }
    else if (strcmp(argv[1], "corrupted") == 0)
    {
        test_corrupted_firmware();
    }
    else
    {
        fprintf(
            stderr,
            "Unknown test: %s\n",
            argv[1]
        );

        return 1;
    }

    return 0;
}