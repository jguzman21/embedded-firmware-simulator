#define _POSIX_C_SOURCE 200809L

#include "message_queue.h"

#include <errno.h>
#include <time.h>

static struct timespec deadline_from_now(uint32_t timeout_ms)
{
    struct timespec deadline;

    clock_gettime(CLOCK_REALTIME, &deadline);

    deadline.tv_sec += timeout_ms / 1000;
    deadline.tv_nsec +=
        (long)(timeout_ms % 1000) * 1000000L;

    if (deadline.tv_nsec >= 1000000000L)
    {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000L;
    }

    return deadline;
}

bool sensor_queue_init(SensorQueue *queue)
{
    if (queue == NULL)
    {
        return false;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        return false;
    }

    if (pthread_cond_init(&queue->not_empty, NULL) != 0)
    {
        pthread_mutex_destroy(&queue->mutex);
        return false;
    }

    if (pthread_cond_init(&queue->not_full, NULL) != 0)
    {
        pthread_cond_destroy(&queue->not_empty);
        pthread_mutex_destroy(&queue->mutex);
        return false;
    }

    return true;
}

void sensor_queue_destroy(SensorQueue *queue)
{
    if (queue == NULL)
    {
        return;
    }

    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
    pthread_mutex_destroy(&queue->mutex);
}

bool sensor_queue_push(
    SensorQueue *queue,
    const SensorReading *reading)
{
    if (queue == NULL || reading == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&queue->mutex);

    if (queue->count >= SENSOR_QUEUE_CAPACITY)
    {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }

    queue->buffer[queue->tail] = *reading;

    queue->tail =
        (queue->tail + 1) % SENSOR_QUEUE_CAPACITY;

    queue->count++;

    pthread_cond_signal(&queue->not_empty);

    pthread_mutex_unlock(&queue->mutex);

    return true;
}

bool sensor_queue_pop(
    SensorQueue *queue,
    SensorReading *reading,
    uint32_t timeout_ms)
{
    if (queue == NULL || reading == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0)
    {
        struct timespec deadline =
            deadline_from_now(timeout_ms);

        int result = pthread_cond_timedwait(
            &queue->not_empty,
            &queue->mutex,
            &deadline
        );

        if (result == ETIMEDOUT)
        {
            pthread_mutex_unlock(&queue->mutex);
            return false;
        }

        if (result != 0)
        {
            pthread_mutex_unlock(&queue->mutex);
            return false;
        }
    }

    *reading = queue->buffer[queue->head];

    queue->head =
        (queue->head + 1) % SENSOR_QUEUE_CAPACITY;

    queue->count--;

    pthread_cond_signal(&queue->not_full);

    pthread_mutex_unlock(&queue->mutex);

    return true;
}