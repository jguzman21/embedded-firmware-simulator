# Testing

## Testing Strategy

Testing is divided into:

1. **Automated unit tests** for firmware validation.
2. **Manual integration scenarios** for bootloader, firmware-update, rollback, watchdog, and recovery behavior.

The automated suite uses CTest.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Automated Unit Tests

Run:

```bash
ctest --test-dir build --output-on-failure
```

The current suite contains four independently reported tests:

```text
FirmwareValidation_ValidImage
FirmwareValidation_InvalidMagic
FirmwareValidation_InvalidSize
FirmwareValidation_CorruptedData
```

### Valid Image

Creates a valid firmware image and verifies that validation accepts it.

```text
Valid firmware -> PASS
```

### Invalid Magic

Replaces the expected firmware magic number with an invalid value.

```text
Invalid magic -> REJECT
```

### Invalid Size

Sets the firmware size above the configured maximum.

```text
Invalid size -> REJECT
```

### Corrupted Data

Modifies firmware data after its CRC32 has been calculated.

```text
CRC mismatch -> REJECT
```

## Manual Integration Tests

### 1. Normal Boot and Firmware Update

```bash
./build/firmware_sim
```

Expected flow:

```text
Bootloader starts
 -> Current firmware validated
 -> New firmware detected
 -> Candidate validated
 -> Update installed
 -> Startup verification passes
 -> New firmware boots
 -> Application starts
```

### 2. Corrupted Firmware Update

The staged firmware can be deliberately modified after its CRC is calculated.

Expected flow:

```text
Candidate detected
 -> CRC validation fails
 -> Update rejected
 -> Current firmware remains active
```

### 3. Failed Firmware Startup and Rollback

The simulator can force startup verification of the newly installed firmware to fail.

Expected flow:

```text
New firmware installed
 -> Startup verification fails
 -> Rollback
 -> Previous firmware restored
 -> Previous firmware boots
```

### 4. Normal Concurrent Application

The normal simulator run should show the temperature, IMU, battery, and health-monitoring tasks operating concurrently. Sensor readings flow through the message queue, watchdog heartbeats are maintained, and the application completes its configured runtime without a watchdog fault.

Because tasks execute concurrently, console output ordering may vary between runs.

### 5. IMU Watchdog Failure and Recovery

Run:

```bash
./build/firmware_sim --hang-imu
```

Expected flow:

```text
IMU task hangs
 -> IMU heartbeat stops
 -> Watchdog timeout
 -> System fault detected
 -> Application performs controlled shutdown
 -> Simulated system reset
 -> Bootloader starts again
 -> Application runs normally
```

## Current Coverage

| Area | Automated coverage |
| --- | --- |
| Valid firmware | Yes |
| Invalid magic | Yes |
| Invalid size | Yes |
| CRC corruption | Yes |
| Bootloader state transitions | Manual integration |
| Firmware update | Manual integration |
| Firmware rollback | Manual integration |
| Concurrent task execution | Manual integration |
| Message queue behavior | Manual integration |
| Watchdog timeout | Manual integration |
| Simulated reset/recovery | Manual integration |

## Interpreting Results

A passing unit-test suite confirms that the tested firmware-validation cases behave as expected. It does not establish that the complete simulator is defect-free.

The integration scenarios are currently verified through observable state transitions and console output.

## Future Testing Extensions

Potential extensions include automated bootloader state-transition tests, automated update/rollback tests, direct message-queue tests, automated watchdog tests, and sanitizer/concurrency-analysis builds.
