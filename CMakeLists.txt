cmake_minimum_required(VERSION 3.20)

project(embedded_firmware_simulator
    VERSION 1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(firmware_sim
    src/main.c
    src/firmware.c
    src/crc32.c
)

target_include_directories(firmware_sim
    PRIVATE
        include
)