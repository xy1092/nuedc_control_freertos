#include "dev_line_sensor.h"
#include <limits.h>
#include <string.h>

static uint8_t clamp_count(uint8_t count)
{
    return (count > DRV_LINE_SENSOR_MAX_COUNT) ? DRV_LINE_SENSOR_MAX_COUNT : count;
}

static uint8_t get_count(const DevLineSensor *sensor)
{
    if (sensor == 0) {
        return 0u;
    }

    if (sensor->has_array) {
        return Drv_LineSensor_Count(&sensor->array_storage);
    }

    return clamp_count(sensor->config.count);
}

static float get_alpha(const DevLineSensor *sensor)
{
    float alpha = sensor->config.bias_alpha;

    if (alpha <= 0.0f || alpha > 1.0f) {
        alpha = DEV_LINE_SENSOR_DEFAULT_BIAS_ALPHA;
    }
    return alpha;
}

static uint16_t get_contrast_threshold(const DevLineSensor *sensor)
{
    return (sensor->config.contrast_threshold != 0u) ?
        sensor->config.contrast_threshold : DEV_LINE_SENSOR_DEFAULT_CONTRAST_TH;
}

static uint16_t get_strength_threshold(const DevLineSensor *sensor)
{
    return (sensor->config.strength_threshold != 0u) ?
        sensor->config.strength_threshold : DEV_LINE_SENSOR_DEFAULT_STRENGTH_TH;
}

static int16_t sensor_weight(uint8_t index, uint8_t count)
{
    return (int16_t)((2 * (int16_t)index) - ((int16_t)count - 1));
}

static uint16_t clamp_u16(uint32_t value)
{
    return (value > UINT16_MAX) ? UINT16_MAX : (uint16_t)value;
}

static void clear_unused_raw(DevLineSensorFrame *frame, uint8_t count)
{
    for (uint8_t i = count; i < DRV_LINE_SENSOR_MAX_COUNT; ++i) {
        frame->raw[i] = 0u;
    }
}

static uint8_t bits_from_raw(const DevLineSensorFrame *frame)
{
    uint8_t bits = 0u;
    uint16_t threshold;

    if (frame->count == 0u || frame->contrast == 0u) {
        return 0u;
    }

    threshold = (uint16_t)(frame->min_value + (frame->contrast / 2u));
    for (uint8_t i = 0; i < frame->count; ++i) {
        if (frame->raw[i] >= threshold) {
            bits |= (uint8_t)(1u << i);
        }
    }
    return bits;
}

void Dev_LineSensor_Init(DevLineSensor *sensor, const DevLineSensorConfig *config)
{
    if (sensor == 0) {
        return;
    }

    memset(sensor, 0, sizeof(*sensor));
    if (config != 0) {
        sensor->config = *config;
        if (config->array != 0) {
            sensor->array_storage = *config->array;
            sensor->config.array = &sensor->array_storage;
            sensor->has_array = 1u;
        }
    }

    sensor->frame.count = get_count(sensor);
}

void Dev_LineSensor_Reset(DevLineSensor *sensor)
{
    if (sensor == 0) {
        return;
    }

    memset(&sensor->frame, 0, sizeof(sensor->frame));
    sensor->frame.count = get_count(sensor);
    sensor->bias_primed = 0u;
}

const DevLineSensorFrame *Dev_LineSensor_Update(DevLineSensor *sensor)
{
    DevLineSensorFrame *frame;
    uint8_t count;
    uint16_t min_v;
    uint16_t max_v;
    uint32_t strength = 0u;
    int32_t weighted = 0;

    if (sensor == 0) {
        return 0;
    }

    frame = &sensor->frame;
    count = get_count(sensor);
    frame->count = count;
    frame->bits = 0u;

    if (count == 0u) {
        clear_unused_raw(frame, 0u);
        frame->min_value = 0u;
        frame->max_value = 0u;
        frame->contrast = 0u;
        frame->strength = 0u;
        frame->on_line = 0u;
        return frame;
    }

    if (sensor->config.read_raw != 0) {
        sensor->config.read_raw(sensor->config.context, frame->raw, count);
    } else if (sensor->has_array) {
        Drv_LineSensor_ReadRaw(&sensor->array_storage, frame->raw, count);
    }
    clear_unused_raw(frame, count);

    min_v = frame->raw[0];
    max_v = frame->raw[0];
    for (uint8_t i = 0; i < count; ++i) {
        if (frame->raw[i] < min_v) {
            min_v = frame->raw[i];
        }
        if (frame->raw[i] > max_v) {
            max_v = frame->raw[i];
        }
    }

    for (uint8_t i = 0; i < count; ++i) {
        uint16_t v = (uint16_t)(frame->raw[i] - min_v);
        strength += v;
        weighted += (int32_t)sensor_weight(i, count) * (int32_t)v;
    }

    frame->min_value = min_v;
    frame->max_value = max_v;
    frame->contrast = (uint16_t)(max_v - min_v);
    frame->strength = clamp_u16(strength);
    frame->on_line = (frame->contrast >= get_contrast_threshold(sensor) &&
                      frame->strength >= get_strength_threshold(sensor)) ? 1u : 0u;
    frame->bits = sensor->has_array ? Drv_LineSensor_ReadBits(&sensor->array_storage) :
                  bits_from_raw(frame);

    if (frame->on_line && strength != 0u && count > 1u) {
        float span = (float)(count - 1u);
        float raw_bias = (float)weighted / ((float)strength * span);

        if (!sensor->bias_primed) {
            frame->bias = raw_bias;
            sensor->bias_primed = 1u;
        } else {
            float alpha = get_alpha(sensor);
            frame->bias = alpha * raw_bias + (1.0f - alpha) * frame->bias;
        }
        frame->last_bias = frame->bias;
    } else {
        frame->bias = frame->last_bias;
    }

    return frame;
}

const DevLineSensorFrame *Dev_LineSensor_GetFrame(const DevLineSensor *sensor)
{
    return (sensor != 0) ? &sensor->frame : 0;
}
