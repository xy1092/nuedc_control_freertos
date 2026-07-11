#ifndef DEV_LINE_SENSOR_H
#define DEV_LINE_SENSOR_H

#include <stdint.h>
#include "drv_line_sensor.h"

#define DEV_LINE_SENSOR_DEFAULT_CONTRAST_TH 120u
#define DEV_LINE_SENSOR_DEFAULT_STRENGTH_TH 260u
#define DEV_LINE_SENSOR_DEFAULT_BIAS_ALPHA  0.35f

typedef void (*DevLineSensorReadRawFn)(void *context, uint16_t *raw, uint8_t count);

typedef struct {
    const DrvLineSensorArray *array;
    DevLineSensorReadRawFn read_raw;
    void *context;
    uint8_t count;
    uint16_t contrast_threshold;
    uint16_t strength_threshold;
    float bias_alpha;
} DevLineSensorConfig;

typedef struct {
    uint16_t raw[DRV_LINE_SENSOR_MAX_COUNT];
    uint8_t count;
    uint8_t bits;
    uint16_t min_value;
    uint16_t max_value;
    uint16_t contrast;
    uint16_t strength;
    float bias;
    float last_bias;
    uint8_t on_line;
} DevLineSensorFrame;

typedef struct {
    DevLineSensorConfig config;
    DrvLineSensorArray array_storage;
    DevLineSensorFrame frame;
    uint8_t has_array;
    uint8_t bias_primed;
} DevLineSensor;

void Dev_LineSensor_Init(DevLineSensor *sensor, const DevLineSensorConfig *config);
void Dev_LineSensor_Reset(DevLineSensor *sensor);
const DevLineSensorFrame *Dev_LineSensor_Update(DevLineSensor *sensor);
const DevLineSensorFrame *Dev_LineSensor_GetFrame(const DevLineSensor *sensor);

#endif
