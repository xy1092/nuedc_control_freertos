#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

typedef struct {
    float alpha;
    float y;
    uint8_t primed;
} LPF1_t;

void  LPF1_Init(LPF1_t *f, float alpha);
float LPF1_Update(LPF1_t *f, float x);

typedef struct {
    int32_t buf[16];
    uint8_t idx;
    uint8_t len;
    int32_t sum;
} SMA_t;

void  SMA_Init(SMA_t *s, uint8_t len);
int32_t SMA_Update(SMA_t *s, int32_t x);

#endif
