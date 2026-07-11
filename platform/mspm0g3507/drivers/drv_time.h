#ifndef DRV_TIME_H
#define DRV_TIME_H

#include <stdint.h>

uint32_t Drv_Time_Millis(void);
uint8_t Drv_Time_HasElapsed(uint32_t start_ms, uint32_t period_ms);

#endif
