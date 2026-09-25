#define _POSIX_C_SOURCE 200809L

#include "application.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#include "health_monitor.h"
#include "message_queue.h"
#include "sensor.h"
#include "watchdog.h"

#define TEMPERATURE_PERIOD_MS 500U
#define IMU_PERIOD_MS         100U
#define BATTERY_PERIOD_MS     1000U

#define WATCHDOG_TIMEOUT_MS   750U

typedef struct
{
    SensorQueue queue;
    Watchdog watchdog;

    atomic_bool stop_requested;

    bool simulate_imu_hang;

    uint64_t start_time_ms;

    pthread_t temperature_thread;
    pthread_t imu_thread;
    pthread_t battery_thread;
    pthread_t health_thread;

    bool temperature_started;
    bool imu_started;
    bool battery_started;
    bool health_started;
} ApplicationContext;

typedef struct
{
    ApplicationContext *application;

    SensorType sensor_type;
    TaskId task_id;
    uint32_t period_ms;
} SensorTaskArguments;

typedef struct
{
    ApplicationContext *application;
} HealthTaskArguments;

static uint64_t monotonic_time_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ((uint64_t)ts.tv_sec * 1000ULL) +
           ((uint64_t)ts.tv_nsec / 1000000ULL);
}

static void sleep_ms(uint32_t milliseconds)
{
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec =
        (long)(milliseconds % 1000) * 1000000L;

    nanosleep(&ts, NULL);
}

static uint32_t application_timestamp_ms(
    ApplicationContext *application)
{
    return (uint32_t)(
        monotonic_time_ms() -
        application->start_time_ms
    );
}

static bool application_should_stop(
    ApplicationContext *application)
{
    return atomic_load(
               &application->stop_requested
           ) ||
           watchdog_is_fault(
               &application->watchdog
           );
}

static void *sensor_task(void *argument)
{
    SensorTaskArguments *task = argument;

    ApplicationContext *application =
        task->application;

    printf(
        "[TASK] %s started\n",
        sensor_name(task->sensor_type)
    );

    fflush(stdout);

    uint32_t time_since_read_ms = task->period_ms;

    while (!application_should_stop(application))
    {
        /*
         * Deliberately simulate an IMU task that
         * becomes stuck and stops reporting heartbeats.
         */
        if (task->sensor_type == SENSOR_IMU &&
            application->simulate_imu_hang)
        {
            printf(
                "[TASK][IMU] Simulated task hang!\n"
            );

            printf(
                "[TASK][IMU] Heartbeats suspended\n"
            );

            fflush(stdout);

            while (!application_should_stop(application))
            {
                sleep_ms(100);
            }

            break;
        }

        /*
         * The task remains alive every 100 ms,
         * regardless of how often it reads its sensor.
         */
        watchdog_heartbeat(
            &application->watchdog,
            task->task_id
        );

        /*
         * Perform the actual sensor reading only
         * when the sensor's configured period expires.
         */
        if (time_since_read_ms >= task->period_ms)
        {
            uint32_t timestamp =
                application_timestamp_ms(application);

            SensorReading reading;

            if (sensor_read(
                    task->sensor_type,
                    timestamp,
                    &reading))
            {
                if (sensor_queue_push(
                        &application->queue,
                        &reading))
                {
                    printf(
                        "[TASK][%s][%u ms] "
                        "Reading: %.2f\n",
                        sensor_name(task->sensor_type),
                        timestamp,
                        reading.value
                    );

                    fflush(stdout);
                }
                else
                {
                    printf(
                        "[TASK][%s] Queue full - "
                        "reading dropped\n",
                        sensor_name(task->sensor_type)
                    );

                    fflush(stdout);
                }
            }

            time_since_read_ms = 0;
        }

        sleep_ms(100);
        time_since_read_ms += 100;
    }

    printf(
        "[TASK] %s stopped\n",
        sensor_name(task->sensor_type)
    );

    fflush(stdout);

    return NULL;
}

static void *health_task(void *argument)
{
    HealthTaskArguments *task = argument;

    ApplicationContext *application =
        task->application;

    SensorReading temperature = {0};
    SensorReading imu = {0};
    SensorReading battery = {0};

    bool have_temperature = false;
    bool have_imu = false;
    bool have_battery = false;

    SystemHealth previous_health =
        SYSTEM_HEALTH_OK;

    bool have_health = false;

    uint64_t last_health_report_ms = 0;

    printf("[TASK] HEALTH_MONITOR started\n");
    fflush(stdout);

    while (!application_should_stop(application))
    {
        SensorReading reading;

        if (sensor_queue_pop(
                &application->queue,
                &reading,
                100))
        {
            switch (reading.type)
            {
                case SENSOR_TEMPERATURE:
                    temperature = reading;
                    have_temperature = true;
                    break;

                case SENSOR_IMU:
                    imu = reading;
                    have_imu = true;
                    break;

                case SENSOR_BATTERY:
                    battery = reading;
                    have_battery = true;
                    break;

                default:
                    break;
            }

            if (have_temperature &&
                have_imu &&
                have_battery)
            {
                SystemHealth current_health =
                    health_monitor_check(
                        &temperature,
                        &imu,
                        &battery
                    );

                uint64_t now =
                    application_timestamp_ms(
                        application
                    );

                if (!have_health ||
                    current_health != previous_health ||
                    now - last_health_report_ms >= 500)
                {
                    printf(
                        "[HEALTH][%u ms] "
                        "System status: %s\n",
                        (unsigned int)now,
                        health_status_name(
                            current_health
                        )
                    );

                    fflush(stdout);

                    previous_health =
                        current_health;

                    last_health_report_ms = now;
                    have_health = true;
                }
            }
        }

        watchdog_heartbeat(
            &application->watchdog,
            TASK_HEALTH
        );
    }

    printf("[TASK] HEALTH_MONITOR stopped\n");
    fflush(stdout);

    return NULL;
}

void application_run(
    uint32_t duration_ms,
    bool simulate_imu_hang)
{
    ApplicationContext application = {0};

    atomic_init(
        &application.stop_requested,
        false
    );

    application.simulate_imu_hang =
        simulate_imu_hang;

    application.start_time_ms =
        monotonic_time_ms();

    if (!sensor_queue_init(&application.queue))
    {
        printf(
            "[APP] Failed to initialize sensor queue\n"
        );

        return;
    }

    if (!watchdog_init(
            &application.watchdog,
            WATCHDOG_TIMEOUT_MS))
    {
        printf(
            "[APP] Failed to initialize watchdog\n"
        );

        sensor_queue_destroy(&application.queue);
        return;
    }

    printf("\n");
    printf("=================================\n");
    printf(" Concurrent Application Started\n");
    printf("=================================\n\n");

    watchdog_start(&application.watchdog);

    SensorTaskArguments temperature_args =
    {
        .application = &application,
        .sensor_type = SENSOR_TEMPERATURE,
        .task_id = TASK_TEMPERATURE,
        .period_ms = TEMPERATURE_PERIOD_MS
    };

    SensorTaskArguments imu_args =
    {
        .application = &application,
        .sensor_type = SENSOR_IMU,
        .task_id = TASK_IMU,
        .period_ms = IMU_PERIOD_MS
    };

    SensorTaskArguments battery_args =
    {
        .application = &application,
        .sensor_type = SENSOR_BATTERY,
        .task_id = TASK_BATTERY,
        .period_ms = BATTERY_PERIOD_MS
    };

    HealthTaskArguments health_args =
    {
        .application = &application
    };

    if (pthread_create(
            &application.temperature_thread,
            NULL,
            sensor_task,
            &temperature_args) == 0)
    {
        application.temperature_started = true;
    }

    if (pthread_create(
            &application.imu_thread,
            NULL,
            sensor_task,
            &imu_args) == 0)
    {
        application.imu_started = true;
    }

    if (pthread_create(
            &application.battery_thread,
            NULL,
            sensor_task,
            &battery_args) == 0)
    {
        application.battery_started = true;
    }

    if (pthread_create(
            &application.health_thread,
            NULL,
            health_task,
            &health_args) == 0)
    {
        application.health_started = true;
    }

    uint32_t elapsed_ms = 0;

    while (elapsed_ms < duration_ms &&
           !watchdog_is_fault(&application.watchdog))
    {
        sleep_ms(100);

        elapsed_ms += 100;
    }

    if (watchdog_is_fault(&application.watchdog))
    {
        printf(
            "\n[APP] Watchdog reported a system fault\n"
        );

        printf(
            "[APP] Initiating controlled shutdown\n"
        );

        fflush(stdout);
    }
    else
    {
        printf(
            "\n[APP] Application runtime complete\n"
        );

        fflush(stdout);
    }

    atomic_store(
        &application.stop_requested,
        true
    );

    if (application.temperature_started)
    {
        pthread_join(
            application.temperature_thread,
            NULL
        );
    }

    if (application.imu_started)
    {
        pthread_join(
            application.imu_thread,
            NULL
        );
    }

    if (application.battery_started)
    {
        pthread_join(
            application.battery_thread,
            NULL
        );
    }

    if (application.health_started)
    {
        pthread_join(
            application.health_thread,
            NULL
        );
    }

    watchdog_stop(&application.watchdog);
    watchdog_destroy(&application.watchdog);

    sensor_queue_destroy(&application.queue);

    printf("\n");
    printf("=================================\n");
    printf(" Concurrent Application Stopped\n");
    printf("=================================\n");
}