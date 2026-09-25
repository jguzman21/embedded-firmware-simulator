#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

#include "sensor.h"

#define SENSOR_QUEUE_CAPACITY 32

typedef struct
{
    SensorReading buffer[SENSOR_QUEUE_CAPACITY];

    size_t head;
    size_t tail;
    size_t count;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} SensorQueue;

bool sensor_queue_init(SensorQueue *queue);

void sensor_queue_destroy(SensorQueue *queue);

bool sensor_queue_push(
    SensorQueue *queue,
    const SensorReading *reading
);

bool sensor_queue_pop(
    SensorQueue *queue,
    SensorReading *reading,
    uint32_t timeout_ms
);

#endif