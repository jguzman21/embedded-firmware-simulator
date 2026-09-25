# Embedded Firmware Simulator

A host-based simulation of an embedded firmware platform written in C.

The project models a bootloader that validates firmware integrity, manages simulated firmware updates and rollback, and then hands control to a concurrent application. The application simulates periodic sensor tasks, inter-task communication, health monitoring, and watchdog-based fault detection.

> **Note:** This project is a software simulation. It does not run on physical microcontroller hardware and the sensor/firmware storage behavior is simulated rather than connected to real peripherals or flash memory.

## Overview

The simulator models the lifecycle of an embedded device:

1. Start in the bootloader.
2. Validate the currently installed firmware.
3. Check for a newer staged firmware image.
4. Validate and install the update when appropriate.
5. Verify the updated firmware startup.
6. Roll back to the previous firmware if startup verification fails.
7. Start the application.
8. Run concurrent sensor tasks and a health-monitoring task.
9. Use a watchdog to detect a hung task.
10. Simulate a system reset after a watchdog fault.

## Architecture

```text
                         POWER ON
                            |
                            v
                    +---------------+
                    |  BOOTLOADER   |
                    +-------+-------+
                            |
                    Validate firmware
                            |
                    Check for update
                            |
                 +----------+----------+
                 |                     |
            No update               Update
                 |                     |
                 |              Validate candidate
                 |                     |
                 |                +----+----+
                 |                |         |
                 |              PASS      FAIL
                 |                |         |
                 |            Install       |
                 |                |         |
                 |             Verify       |
                 |                |         |
                 |             +--+--+      |
                 |             |     |      |
                 |           PASS   FAIL    |
                 |             |     |      |
                 |             |  Rollback  |
                 |             |     |      |
                 +-------------+-----+------+
                               |
                               v
                    +---------------------+
                    |     APPLICATION     |
                    +----------+----------+
                               |
          +--------------------+--------------------+
          |                    |                    |
          v                    v                    v
   Temperature Task       IMU Task           Battery Task
          |                    |                    |
          +--------------------+--------------------+
                               |
                               v
                      Sensor Message Queue
                               |
                               v
                      Health Monitor Task
                               ^
                               |
                           Watchdog
                               |
                         Task heartbeat
                               |
                         Timeout / fault
                               |
                               v
                         SYSTEM RESET
                               |
                               v
                          BOOTLOADER
```

## Key Components

### Bootloader

The bootloader is implemented as a finite state machine with states for:

- Reset
- Self-test
- Firmware validation
- Update detection
- Update validation
- Update installation
- Startup verification
- Rollback
- Recovery
- Application boot

### Firmware Validation

Each simulated firmware image contains a header with:

- Magic number
- Firmware version
- Firmware size
- CRC32
- Firmware data

The bootloader validates the image before allowing it to boot.

CRC32 verification is also used to detect corrupted firmware data.

### Firmware Updates and Rollback

The simulator supports a staged firmware image.

A newer candidate firmware is:

1. Detected.
2. Validated.
3. Installed.
4. Startup-verified.

A failed startup verification causes the simulator to restore the previous firmware image.

### Concurrent Application

The application uses POSIX threads to model independent embedded tasks:

- Temperature task
- IMU task
- Battery task
- Health-monitoring task

Each sensor task has its own sampling period while continuing to report watchdog heartbeats independently.

### Inter-Task Communication

Sensor tasks publish `SensorReading` structures through a bounded, thread-safe circular queue.

The queue uses:

- `pthread_mutex_t` for mutual exclusion
- Condition variables for blocking/waiting
- A fixed-capacity circular buffer

### Health Monitoring

The health-monitoring task consumes sensor readings and reports:

- `OK`
- `WARNING`
- `FAULT`

The current simulation checks conditions such as invalid sensor readings, high temperature, and low battery level.

### Watchdog and Fault Injection

Each task periodically reports a heartbeat to the watchdog.

The watchdog detects a task that stops reporting within the configured timeout.

The simulator provides an IMU fault-injection mode:

```bash
./build/firmware_sim --hang-imu
```

This intentionally suspends IMU heartbeats.

The expected flow is:

```text
IMU task hangs
    |
    v
Watchdog timeout
    |
    v
System fault detected
    |
    v
Controlled application shutdown
    |
    v
Simulated system reset
    |
    v
Bootloader starts again
    |
    v
Application runs normally
```

## Project Structure

```text
embedded-firmware-simulator/
|
+-- include/
|   +-- application.h
|   +-- bootloader.h
|   +-- crc32.h
|   +-- firmware.h
|   +-- health_monitor.h
|   +-- message_queue.h
|   +-- sensor.h
|   +-- watchdog.h
|
+-- src/
|   +-- application.c
|   +-- bootloader.c
|   +-- crc32.c
|   +-- firmware.c
|   +-- health_monitor.c
|   +-- main.c
|   +-- message_queue.c
|   +-- sensor.c
|   +-- watchdog.c
|
+-- tests/
|   +-- test_firmware.c
|
+-- docs/
|
+-- CMakeLists.txt
+-- README.md
+-- .gitignore
```

## Build Requirements

This project is intended to be built in a Linux environment such as Ubuntu or WSL.

Required tools:

- GCC
- CMake
- Git
- POSIX threads
- CTest

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

Run the normal simulation:

```bash
./build/firmware_sim
```

Run the IMU watchdog fault-injection scenario:

```bash
./build/firmware_sim --hang-imu
```

## Tests

The firmware validation tests are registered individually with CTest.

Run the full test suite:

```bash
ctest --test-dir build --output-on-failure
```

The current suite includes:

```text
FirmwareValidation_ValidImage
FirmwareValidation_InvalidMagic
FirmwareValidation_InvalidSize
FirmwareValidation_CorruptedData
```

The tests verify both valid firmware acceptance and rejection of invalid or corrupted images.

## Design Notes

This project intentionally uses a host-based implementation so embedded-system concepts can be developed and tested without physical hardware.

The application uses POSIX threads as a stand-in for independently scheduled RTOS-style tasks. The project does **not** implement or claim to be a real RTOS.

Similarly, firmware installation, rollback, and sensor behavior are simulated in memory rather than performed through a microcontroller's flash controller, hardware peripherals, or physical sensors.

## Technologies

- C11
- GCC
- CMake
- POSIX threads (`pthread`)
- Mutexes
- Condition variables
- CRC32
- CTest
- Git

## Possible Future Extensions

Potential directions for a hardware-based version include:

- Mapping the firmware model to an STM32 flash layout
- Replacing simulated sensors with hardware drivers
- Using FreeRTOS tasks and queues
- Adding persistent boot/update metadata
- Adding a real firmware image file format
- Adding additional fault-injection scenarios
- Expanding automated integration tests

## Author

Jonathan Guzman
