#define _POSIX_C_SOURCE 200809L

#include "watchdog.h"

#include <stdio.h>
#include <time.h>

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

const char *watchdog_task_name(TaskId task)
{
    switch (task)
    {
        case TASK_TEMPERATURE:
            return "TEMPERATURE_TASK";

        case TASK_IMU:
            return "IMU_TASK";

        case TASK_BATTERY:
            return "BATTERY_TASK";

        case TASK_HEALTH:
            return "HEALTH_TASK";

        default:
            return "UNKNOWN_TASK";
    }
}

static void *watchdog_thread(void *argument)
{
    Watchdog *watchdog = argument;

    while (true)
    {
        sleep_ms(100);

        pthread_mutex_lock(&watchdog->mutex);

        if (watchdog->stop_requested)
        {
            pthread_mutex_unlock(&watchdog->mutex);
            break;
        }

        uint64_t now = monotonic_time_ms();

        for (TaskId task = 0;
             task < TASK_COUNT;
             task++)
        {
            uint64_t elapsed =
                now - watchdog->last_heartbeat_ms[task];

            if (elapsed > watchdog->timeout_ms)
            {
                if (!watchdog->fault)
                {
                    printf(
                        "[WATCHDOG] TIMEOUT: %s "
                        "missed heartbeat for %llu ms\n",
                        watchdog_task_name(task),
                        (unsigned long long)elapsed
                    );

                    printf(
                        "[WATCHDOG] SYSTEM FAULT DETECTED\n"
                    );

                    fflush(stdout);

                    watchdog->fault = true;
                }

                break;
            }
        }

        pthread_mutex_unlock(&watchdog->mutex);
    }

    return NULL;
}

bool watchdog_init(
    Watchdog *watchdog,
    uint32_t timeout_ms)
{
    if (watchdog == NULL)
    {
        return false;
    }

    if (pthread_mutex_init(&watchdog->mutex, NULL) != 0)
    {
        return false;
    }

    watchdog->timeout_ms = timeout_ms;
    watchdog->fault = false;
    watchdog->stop_requested = false;
    watchdog->thread_started = false;

    uint64_t now = monotonic_time_ms();

    for (TaskId task = 0;
         task < TASK_COUNT;
         task++)
    {
        watchdog->last_heartbeat_ms[task] = now;
    }

    return true;
}

bool watchdog_start(Watchdog *watchdog)
{
    if (watchdog == NULL)
    {
        return false;
    }

    if (pthread_create(
            &watchdog->monitor_thread,
            NULL,
            watchdog_thread,
            watchdog) != 0)
    {
        return false;
    }

    watchdog->thread_started = true;

    return true;
}

void watchdog_heartbeat(
    Watchdog *watchdog,
    TaskId task)
{
    if (watchdog == NULL ||
        task >= TASK_COUNT)
    {
        return;
    }

    pthread_mutex_lock(&watchdog->mutex);

    watchdog->last_heartbeat_ms[task] =
        monotonic_time_ms();

    pthread_mutex_unlock(&watchdog->mutex);
}

bool watchdog_is_fault(
    Watchdog *watchdog)
{
    if (watchdog == NULL)
    {
        return true;
    }

    pthread_mutex_lock(&watchdog->mutex);

    bool fault = watchdog->fault;

    pthread_mutex_unlock(&watchdog->mutex);

    return fault;
}

void watchdog_stop(Watchdog *watchdog)
{
    if (watchdog == NULL ||
        !watchdog->thread_started)
    {
        return;
    }

    pthread_mutex_lock(&watchdog->mutex);

    watchdog->stop_requested = true;

    pthread_mutex_unlock(&watchdog->mutex);

    pthread_join(
        watchdog->monitor_thread,
        NULL
    );

    watchdog->thread_started = false;
}

void watchdog_destroy(Watchdog *watchdog)
{
    if (watchdog == NULL)
    {
        return;
    }

    pthread_mutex_destroy(&watchdog->mutex);
}