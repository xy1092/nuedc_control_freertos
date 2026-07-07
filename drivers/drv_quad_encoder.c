#include "drv_quad_encoder.h"

static uint8_t read_state(const DrvQuadEncoder *encoder)
{
    uint8_t a = Drv_Gpio_ReadRaw(&encoder->config.phase_a);
    uint8_t b = Drv_Gpio_ReadRaw(&encoder->config.phase_b);

    return (uint8_t)((a << 1) | b);
}

static int8_t decode_step(uint8_t last, uint8_t cur)
{
    static const int8_t table[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0,
    };

    return table[((last & 0x03u) << 2) | (cur & 0x03u)];
}

void Drv_QuadEncoder_Init(DrvQuadEncoder *encoder, const DrvQuadEncoderConfig *config)
{
    if ((encoder == 0) || (config == 0)) {
        return;
    }

    encoder->config = *config;
    if (encoder->config.direction == 0) {
        encoder->config.direction = 1;
    }
    encoder->delta = 0;
    encoder->total = 0;
    encoder->last_state = read_state(encoder);
}

void Drv_QuadEncoder_Poll(DrvQuadEncoder *encoder)
{
    uint8_t cur;
    int32_t step;

    if (encoder == 0) {
        return;
    }

    cur = read_state(encoder);
    step = (int32_t)decode_step(encoder->last_state, cur) * encoder->config.direction;
    encoder->last_state = cur;
    encoder->delta += step;
    encoder->total += step;
}

int32_t Drv_QuadEncoder_GetDelta(DrvQuadEncoder *encoder)
{
    int32_t delta;

    if (encoder == 0) {
        return 0;
    }

    delta = encoder->delta;
    encoder->delta = 0;
    return delta;
}

int64_t Drv_QuadEncoder_GetTotal(const DrvQuadEncoder *encoder)
{
    if (encoder == 0) {
        return 0;
    }

    return encoder->total;
}

void Drv_QuadEncoder_Reset(DrvQuadEncoder *encoder)
{
    if (encoder == 0) {
        return;
    }

    encoder->delta = 0;
    encoder->total = 0;
    encoder->last_state = read_state(encoder);
}
