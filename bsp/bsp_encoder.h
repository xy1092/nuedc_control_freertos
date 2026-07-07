#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

void    BSP_Encoder_Init(void);

/* 高频轮询，建议在主循环每轮调用，用于累计正交编码器变化。 */
void    BSP_Encoder_Poll(void);

/* 控制周期内增量，正负反映方向 */
int32_t BSP_Encoder_GetDelta(uint8_t ch);

/* 累计脉冲数（里程计用） */
int64_t BSP_Encoder_GetTotal(uint8_t ch);

void    BSP_Encoder_Reset(uint8_t ch);

#endif
