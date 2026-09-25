cmake_minimum_required(VERSION 3.20)

project(embedded_firmware_simulator
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(firmware_sim
    src/main.c
    src/firmware.c
    src/crc32.c
    src/bootloader.c
    src/application.c
    src/sensor.c
    src/health_monitor.c
    src/message_queue.c
    src/watchdog.c
)

target_include_directories(firmware_sim
    PRIVATE
        include
)

target_link_libraries(firmware_sim
    PRIVATE
        Threads::Threads
)

enable_testing()

add_executable(test_firmware
    tests/test_firmware.c
    src/firmware.c
    src/crc32.c
)

target_include_directories(test_firmware
    PRIVATE
        include
)

add_test(
    NAME FirmwareValidation_ValidImage
    COMMAND test_firmware valid
)

add_test(
    NAME FirmwareValidation_InvalidMagic
    COMMAND test_firmware invalid_magic
)

add_test(
    NAME FirmwareValidation_InvalidSize
    COMMAND test_firmware invalid_size
)

add_test(
    NAME FirmwareValidation_CorruptedData
    COMMAND test_firmware corrupted
)