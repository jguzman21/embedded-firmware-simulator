#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

typedef enum
{
    TASK_TEMPERATURE,
    TASK_IMU,
    TASK_BATTERY,
    TASK_HEALTH,
    TASK_COUNT
} TaskId;

typedef struct
{
    pthread_mutex_t mutex;

    uint64_t last_heartbeat_ms[TASK_COUNT];

    uint32_t timeout_ms;

    bool fault;
    bool stop_requested;

    pthread_t monitor_thread;
    bool thread_started;
} Watchdog;

bool watchdog_init(
    Watchdog *watchdog,
    uint32_t timeout_ms
);

bool watchdog_start(Watchdog *watchdog);

void watchdog_heartbeat(
    Watchdog *watchdog,
    TaskId task
);

bool watchdog_is_fault(
    Watchdog *watchdog
);

void watchdog_stop(Watchdog *watchdog);

void watchdog_destroy(Watchdog *watchdog);

const char *watchdog_task_name(TaskId task);

#endif