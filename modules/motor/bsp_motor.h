#ifndef BSP_MOTOR_H
#define BSP_MOTOR_H

#include <stdint.h>
#include "dev_motor.h"

/* ch: 0 = 左电机, 1 = 右电机; duty: 0 ~ 1000. 负值会被钳到 0，比赛模式只前进。 */
void BSP_Motor_Init(void);
void BSP_Motor_SetDuty(uint8_t ch, int16_t duty);
void BSP_Motor_SetSignedDuty(uint8_t ch, int16_t duty);
void BSP_Motor_Brake(uint8_t ch);
void BSP_Motor_Standby(uint8_t on);
void BSP_Motor_SafetyTick(uint32_t now_ms);
DevMotor *BSP_Motor_GetDevice(uint8_t ch);

#endif
