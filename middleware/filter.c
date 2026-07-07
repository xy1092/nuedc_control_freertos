#include "filter.h"

void LPF1_Init(LPF1_t *f, float alpha)
{
    f->alpha  = alpha;
    f->y      = 0.0f;
    f->primed = 0;
}

float LPF1_Update(LPF1_t *f, float x)
{
    if (!f->primed) { f->y = x; f->primed = 1; return x; }
    f->y = f->alpha * x + (1.0f - f->alpha) * f->y;
    return f->y;
}

void SMA_Init(SMA_t *s, uint8_t len)
{
    if (len > 16) len = 16;
    s->len = len;
    s->idx = 0;
    s->sum = 0;
    for (uint8_t i = 0; i < 16; ++i) s->buf[i] = 0;
}

int32_t SMA_Update(SMA_t *s, int32_t x)
{
    s->sum -= s->buf[s->idx];
    s->buf[s->idx] = x;
    s->sum += x;
    s->idx = (s->idx + 1) % s->len;
    return s->sum / s->len;
}
