#include "health_monitor.h"

#define HEALTH_STARTUP_GRACE_MS 2000u

static const uint16_t s_deadline_ms[HEALTH_TASK_COUNT] = {
    100u,
    50u,
    100u,
    500u,
};

static volatile uint32_t s_last_beat_ms[HEALTH_TASK_COUNT];
static uint32_t s_started_ms;

void System_Health_Init(uint32_t now_ms)
{
    s_started_ms = now_ms;
    for (uint8_t i = 0u; i < HEALTH_TASK_COUNT; ++i) {
        s_last_beat_ms[i] = now_ms;
    }
}

void System_Health_Beat(HealthTaskId_t task, uint32_t now_ms)
{
    if ((uint32_t)task < (uint32_t)HEALTH_TASK_COUNT) {
        s_last_beat_ms[task] = now_ms;
    }
}

uint8_t System_Health_Check(uint32_t now_ms, uint32_t *stale_mask)
{
    uint32_t mask = 0u;

    if ((uint32_t)(now_ms - s_started_ms) >= HEALTH_STARTUP_GRACE_MS) {
        for (uint8_t i = 0u; i < HEALTH_TASK_COUNT; ++i) {
            if ((uint32_t)(now_ms - s_last_beat_ms[i]) > s_deadline_ms[i]) {
                mask |= (1u << i);
            }
        }
    }
    if (stale_mask != 0) {
        *stale_mask = mask;
    }
    return (mask == 0u) ? 1u : 0u;
}
