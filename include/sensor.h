#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SENSOR_TEMPERATURE,
    SENSOR_IMU,
    SENSOR_BATTERY
} SensorType;

typedef struct
{
    SensorType type;
    bool valid;
    float value;
    uint32_t timestamp_ms;
} SensorReading;

bool sensor_read(
    SensorType type,
    uint32_t timestamp_ms,
    SensorReading *reading
);

const char *sensor_name(SensorType type);

#endif