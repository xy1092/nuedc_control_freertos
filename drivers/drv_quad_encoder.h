#ifndef DRV_QUAD_ENCODER_H
#define DRV_QUAD_ENCODER_H

#include <stdint.h>
#include "drv_gpio.h"

typedef struct {
    DrvGpioPin phase_a;
    DrvGpioPin phase_b;
    int8_t direction;
} DrvQuadEncoderConfig;

typedef struct {
    DrvQuadEncoderConfig config;
    uint8_t last_state;
    int32_t delta;
    int64_t total;
} DrvQuadEncoder;

void Drv_QuadEncoder_Init(DrvQuadEncoder *encoder, const DrvQuadEncoderConfig *config);
void Drv_QuadEncoder_Poll(DrvQuadEncoder *encoder);
int32_t Drv_QuadEncoder_GetDelta(DrvQuadEncoder *encoder);
int64_t Drv_QuadEncoder_GetTotal(const DrvQuadEncoder *encoder);
void Drv_QuadEncoder_Reset(DrvQuadEncoder *encoder);

#endif
