# System Architecture

## Overview

The Embedded Firmware Simulator is a host-based simulation of an embedded firmware platform written in C.

The system is divided into two major phases:

1. **Bootloader** — validates firmware, manages a staged update, and handles rollback/recovery.
2. **Application** — runs concurrent simulated sensor tasks, communicates through a thread-safe queue, monitors system health, and detects task failures with a watchdog.

The simulator intentionally uses software abstractions in place of physical microcontroller hardware, flash memory, and sensors.

## High-Level Architecture

```text
                         POWER ON
                            |
                            v
                    +---------------+
                    |  BOOTLOADER   |
                    +-------+-------+
                            |
                    Validate current FW
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

## Bootloader Architecture

The bootloader is a finite-state machine with states for reset, self-test, firmware validation, update detection, update validation, update installation, startup verification, rollback, recovery, and application boot.

```text
RESET -> SELF_TEST -> VALIDATE_FIRMWARE -> CHECK_FOR_UPDATE
                                             |
                                 +-----------+-----------+
                                 |                       |
                              no update                update
                                 |                       |
                                 v                       v
                          BOOT_APPLICATION       VALIDATE_UPDATE
                                                         |
                                                  +------+------+
                                                  |             |
                                                valid        invalid
                                                  |             |
                                                  v             v
                                           INSTALL_UPDATE   BOOT_APPLICATION
                                                  |
                                                  v
                                        VERIFY_NEW_FIRMWARE
                                             |        |
                                           pass      fail
                                             |        |
                                             v        v
                                      BOOT_APPLICATION ROLLBACK
                                                         |
                                                         v
                                                  BOOT_APPLICATION
```

The bootloader keeps boot/update responsibilities separate from the application.

## Firmware Image

A simulated firmware image contains:

```text
+---------------------------+
| Magic Number              |
+---------------------------+
| Firmware Version          |
+---------------------------+
| Firmware Size             |
+---------------------------+
| CRC32                     |
+---------------------------+
| Firmware Data             |
+---------------------------+
```

The validator checks the expected magic number, a non-zero size within the configured maximum, and a CRC32 calculated over the firmware data.

## Firmware Update and Rollback

The simulator maintains references for current firmware, staged firmware, and previous firmware.

Successful update:

```text
Current v1 -> Candidate v2 -> Validate -> Install -> Verify -> Boot v2
```

Failed startup:

```text
Current v1 -> Install v2 -> Startup verification fails -> Rollback -> Boot v1
```

Firmware installation and rollback are simulated in memory rather than through a real flash controller.

## Application Architecture

The application creates concurrent tasks for temperature, IMU, battery, and health monitoring.

| Task | Sampling period |
| --- | ---: |
| IMU | 100 ms |
| Temperature | 500 ms |
| Battery | 1000 ms |

Sensor tasks report watchdog heartbeats independently of their sensor sampling rates.

The project uses POSIX threads as a host-side model for concurrent RTOS-style tasks; it does not implement an RTOS.

## Inter-Task Communication

```text
Temperature Task ----IMU Task -------------+--> Sensor Queue --> Health Monitor
Battery Task --------/
```

The queue is a fixed-capacity circular buffer protected by a POSIX mutex and coordinated with condition variables.

## Health Monitoring

The health-monitoring task consumes sensor readings and reports `OK`, `WARNING`, or `FAULT`.

The current checks include invalid readings, temperature outside the configured safe range, and low battery level.

## Watchdog

Each task periodically updates a heartbeat timestamp.

```text
Task -> heartbeat timestamp -> Watchdog -> timeout -> SYSTEM FAULT
```

The watchdog timeout is 750 ms. The heartbeat cadence is intentionally independent from the sensor sampling period.

## Fault Injection and Recovery

Run:

```bash
./build/firmware_sim --hang-imu
```

The IMU task intentionally suspends heartbeats. The watchdog detects the timeout, the application shuts down, and the simulator performs a simulated reset before returning through the bootloader.

```text
IMU hangs -> heartbeat stops -> watchdog timeout -> shutdown
         -> simulated reset -> bootloader -> application
```

## Module Responsibilities

| Module | Responsibility |
| --- | --- |
| `main.c` | Creates firmware images, starts the bootloader, handles simulated reset/restart |
| `bootloader.c` | Boot state machine, validation, updates, rollback, recovery |
| `firmware.c` | Firmware validation and metadata display |
| `crc32.c` | CRC32 calculation |
| `application.c` | Application lifecycle, task creation/shutdown, runtime coordination |
| `sensor.c` | Simulated sensor behavior |
| `health_monitor.c` | Sensor health evaluation |
| `message_queue.c` | Thread-safe sensor message queue |
| `watchdog.c` | Heartbeats and watchdog monitoring |

## Design Considerations

The architecture emphasizes separation of concerns: firmware validation is separated from boot-state control; sensor generation is separated from health evaluation; inter-task communication is centralized in a queue; and watchdog monitoring is independent of the sensor tasks.

## Relationship to a Real Embedded System

The project is a software simulation. A future hardware version could map simulated firmware data to MCU flash, simulated sensors to hardware drivers, POSIX threads to RTOS tasks, the message queue to an RTOS queue, and the simulated reset to a real MCU watchdog/system reset.
