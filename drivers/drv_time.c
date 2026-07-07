#include "drv_time.h"

extern volatile uint32_t g_tick_ms;

uint32_t Drv_Time_Millis(void)
{
    return g_tick_ms;
}

uint8_t Drv_Time_HasElapsed(uint32_t start_ms, uint32_t period_ms)
{
    return ((uint32_t)(Drv_Time_Millis() - start_ms) >= period_ms) ? 1u : 0u;
}
