#include "application.h"

#include <stdio.h>

#include "health_monitor.h"
#include "sensor.h"

#define APPLICATION_TICK_MS        100U
#define TEMPERATURE_PERIOD_MS      500U
#define IMU_PERIOD_MS              100U
#define BATTERY_PERIOD_MS          1000U
#define HEALTH_MONITOR_PERIOD_MS   500U

void application_run(uint32_t duration_ms)
{
    SensorReading temperature = {0};
    SensorReading imu = {0};
    SensorReading battery = {0};

    SystemHealth health = SYSTEM_HEALTH_OK;

    printf("=================================\n");
    printf(" Application Started\n");
    printf("=================================\n\n");

    for (uint32_t timestamp_ms = 0;
         timestamp_ms < duration_ms;
         timestamp_ms += APPLICATION_TICK_MS)
    {
        /*
         * Temperature task
         */
        if (timestamp_ms % TEMPERATURE_PERIOD_MS == 0)
        {
            if (sensor_read(
                    SENSOR_TEMPERATURE,
                    timestamp_ms,
                    &temperature))
            {
                printf(
                    "[APP][%5u ms] Temperature: %.2f C\n",
                    timestamp_ms,
                    temperature.value
                );
            }
        }

        /*
         * IMU task
         */
        if (timestamp_ms % IMU_PERIOD_MS == 0)
        {
            if (sensor_read(
                    SENSOR_IMU,
                    timestamp_ms,
                    &imu))
            {
                printf(
                    "[APP][%5u ms] IMU: %.3f g\n",
                    timestamp_ms,
                    imu.value
                );
            }
        }

        /*
         * Battery task
         */
        if (timestamp_ms % BATTERY_PERIOD_MS == 0)
        {
            if (sensor_read(
                    SENSOR_BATTERY,
                    timestamp_ms,
                    &battery))
            {
                printf(
                    "[APP][%5u ms] Battery: %.2f %%\n",
                    timestamp_ms,
                    battery.value
                );
            }
        }

        /*
         * Health monitoring task
         */
        if (timestamp_ms % HEALTH_MONITOR_PERIOD_MS == 0)
        {
            health = health_monitor_check(
                &temperature,
                &imu,
                &battery
            );

            printf(
                "[APP][%5u ms] System Health: %s\n",
                timestamp_ms,
                health_status_name(health)
            );
        }
    }

    printf("\n=================================\n");
    printf(" Application Shutdown\n");
    printf(" Final Health: %s\n",
           health_status_name(health));
    printf("=================================\n");
}