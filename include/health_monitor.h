#ifndef HEALTH_MONITOR_H
#define HEALTH_MONITOR_H

#include <stdbool.h>

#include "sensor.h"

typedef enum
{
    SYSTEM_HEALTH_OK,
    SYSTEM_HEALTH_WARNING,
    SYSTEM_HEALTH_FAULT
} SystemHealth;

SystemHealth health_monitor_check(
    const SensorReading *temperature,
    const SensorReading *imu,
    const SensorReading *battery
);

const char *health_status_name(SystemHealth status);

#endif