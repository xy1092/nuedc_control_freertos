#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include <stdint.h>

void    BSP_Encoder_Init(void);

/* Snapshots the native TIMG8 QEI counter; TIMG6 is accumulated in its ISR. */
void    BSP_Encoder_Poll(void);

/* Delta in the current control interval; sign reflects direction. */
int32_t BSP_Encoder_GetDelta(uint8_t ch);

/* Total edge count for odometry. */
int64_t BSP_Encoder_GetTotal(uint8_t ch);

void    BSP_Encoder_Reset(uint8_t ch);

#endif
