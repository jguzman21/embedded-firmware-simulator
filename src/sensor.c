#include "sensor.h"
#include <stddef.h>

static float simulate_temperature(uint32_t timestamp_ms)
{
    /*
     * Simulate a temperature varying between
     * approximately 24 and 26 degrees Celsius.
     */
    return 24.0f + ((timestamp_ms % 1000) / 1000.0f) * 2.0f;
}

static float simulate_imu(uint32_t timestamp_ms)
{
    /*
     * Simulate the magnitude of an IMU reading
     * near 1 g.
     */
    return 1.0f + ((timestamp_ms % 500) / 5000.0f);
}

static float simulate_battery(uint32_t timestamp_ms)
{
    /*
     * Slowly decrease battery level over time.
     */
    return 100.0f - (timestamp_ms / 1000.0f) * 0.05f;
}

bool sensor_read(
    SensorType type,
    uint32_t timestamp_ms,
    SensorReading *reading)
{
    if (reading == NULL)
    {
        return false;
    }

    reading->type = type;
    reading->timestamp_ms = timestamp_ms;
    reading->valid = true;

    switch (type)
    {
        case SENSOR_TEMPERATURE:
            reading->value =
                simulate_temperature(timestamp_ms);
            break;

        case SENSOR_IMU:
            reading->value =
                simulate_imu(timestamp_ms);
            break;

        case SENSOR_BATTERY:
            reading->value =
                simulate_battery(timestamp_ms);
            break;

        default:
            reading->valid = false;
            reading->value = 0.0f;
            return false;
    }

    return true;
}

const char *sensor_name(SensorType type)
{
    switch (type)
    {
        case SENSOR_TEMPERATURE:
            return "TEMPERATURE";

        case SENSOR_IMU:
            return "IMU";

        case SENSOR_BATTERY:
            return "BATTERY";

        default:
            return "UNKNOWN";
    }
}