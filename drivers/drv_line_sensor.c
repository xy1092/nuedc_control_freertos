#include "drv_line_sensor.h"

uint8_t Drv_LineSensor_Count(const DrvLineSensorArray *array)
{
    if (array == 0) {
        return 0u;
    }

    return (array->count > DRV_LINE_SENSOR_MAX_COUNT) ?
        DRV_LINE_SENSOR_MAX_COUNT : array->count;
}

uint8_t Drv_LineSensor_ReadBits(const DrvLineSensorArray *array)
{
    uint8_t bits = 0u;
    uint8_t count = Drv_LineSensor_Count(array);

    for (uint8_t i = 0; i < count; ++i) {
        if (Drv_Gpio_ReadActive(&array->channels[i])) {
            bits |= (uint8_t)(1u << i);
        }
    }

    return bits;
}

void Drv_LineSensor_ReadRaw(const DrvLineSensorArray *array, uint16_t *raw, uint8_t raw_count)
{
    uint8_t count = Drv_LineSensor_Count(array);

    if (raw == 0) {
        return;
    }

    if (count > raw_count) {
        count = raw_count;
    }

    for (uint8_t i = 0; i < count; ++i) {
        raw[i] = Drv_Gpio_ReadActive(&array->channels[i]) ?
            array->active_raw : array->inactive_raw;
    }
}
