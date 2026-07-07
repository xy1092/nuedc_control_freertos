#include "bsp_encoder.h"
#include "bsp_motor.h"

static inline uint8_t encoder_index(uint8_t ch)
{
    return (ch < 2u) ? ch : 1u;
}

void BSP_Encoder_Init(void)
{
    Dev_Motor_ResetEncoder(BSP_Motor_GetDevice(0));
    Dev_Motor_ResetEncoder(BSP_Motor_GetDevice(1));
}

void BSP_Encoder_Poll(void)
{
    for (uint8_t ch = 0; ch < 2u; ++ch) {
        Dev_Motor_PollEncoder(BSP_Motor_GetDevice(ch));
    }
}

int32_t BSP_Encoder_GetDelta(uint8_t ch)
{
    return Dev_Motor_GetEncoderDelta(BSP_Motor_GetDevice(encoder_index(ch)));
}

int64_t BSP_Encoder_GetTotal(uint8_t ch)
{
    return Dev_Motor_GetEncoderTotal(BSP_Motor_GetDevice(encoder_index(ch)));
}

void BSP_Encoder_Reset(uint8_t ch)
{
    Dev_Motor_ResetEncoder(BSP_Motor_GetDevice(encoder_index(ch)));
}
