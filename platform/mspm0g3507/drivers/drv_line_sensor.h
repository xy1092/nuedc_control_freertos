#ifndef DRV_LINE_SENSOR_H
#define DRV_LINE_SENSOR_H

#include <stdint.h>
#include "drv_gpio.h"

#define DRV_LINE_SENSOR_MAX_COUNT 8u

typedef struct {
    uint8_t count;
    uint16_t active_raw;
    uint16_t inactive_raw;
    DrvGpioPin channels[DRV_LINE_SENSOR_MAX_COUNT];
} DrvLineSensorArray;

uint8_t Drv_LineSensor_Count(const DrvLineSensorArray *array);
uint8_t Drv_LineSensor_ReadBits(const DrvLineSensorArray *array);
void Drv_LineSensor_ReadRaw(const DrvLineSensorArray *array, uint16_t *raw, uint8_t raw_count);

#endif
