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