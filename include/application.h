#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>
#include <stdint.h>

void application_run(
    uint32_t duration_ms,
    bool simulate_imu_hang
);

#endif