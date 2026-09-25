#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APPLICATION_RESULT_COMPLETED,
    APPLICATION_RESULT_WATCHDOG_FAULT
} ApplicationResult;

ApplicationResult application_run(
    uint32_t duration_ms,
    bool simulate_imu_hang
);

#endif