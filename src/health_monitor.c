#include "health_monitor.h"
#include <stddef.h>

SystemHealth health_monitor_check(
    const SensorReading *temperature,
    const SensorReading *imu,
    const SensorReading *battery)
{
    if (temperature == NULL ||
        imu == NULL ||
        battery == NULL)
    {
        return SYSTEM_HEALTH_FAULT;
    }

    if (!temperature->valid ||
        !imu->valid ||
        !battery->valid)
    {
        return SYSTEM_HEALTH_FAULT;
    }

    if (temperature->value < 0.0f ||
        temperature->value > 70.0f)
    {
        return SYSTEM_HEALTH_WARNING;
    }

    if (battery->value < 20.0f)
    {
        return SYSTEM_HEALTH_WARNING;
    }

    return SYSTEM_HEALTH_OK;
}

const char *health_status_name(SystemHealth status)
{
    switch (status)
    {
        case SYSTEM_HEALTH_OK:
            return "OK";

        case SYSTEM_HEALTH_WARNING:
            return "WARNING";

        case SYSTEM_HEALTH_FAULT:
            return "FAULT";

        default:
            return "UNKNOWN";
    }
}