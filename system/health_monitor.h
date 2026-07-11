#ifndef SYSTEM_HEALTH_MONITOR_H
#define SYSTEM_HEALTH_MONITOR_H

#include <stdint.h>

typedef enum {
    HEALTH_TASK_IMU = 0,
    HEALTH_TASK_FAST_IO,
    HEALTH_TASK_CONTROL,
    HEALTH_TASK_TELEMETRY,
    HEALTH_TASK_COUNT,
} HealthTaskId_t;

void System_Health_Init(uint32_t now_ms);
void System_Health_Beat(HealthTaskId_t task, uint32_t now_ms);
uint8_t System_Health_Check(uint32_t now_ms, uint32_t *stale_mask);

#endif
