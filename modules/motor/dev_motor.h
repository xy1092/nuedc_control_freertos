#ifndef DEV_MOTOR_H
#define DEV_MOTOR_H

#include <stdint.h>
#include "drv_hbridge_motor.h"
#include "drv_quad_encoder.h"

#define DEV_MOTOR_DUTY_MAX_DEFAULT 1000

typedef struct {
    DrvHBridgeMotor output;
    DrvQuadEncoderConfig encoder;
    int16_t max_duty;
    uint8_t has_encoder;
} DevMotorConfig;

typedef struct {
    DevMotorConfig config;
    DrvQuadEncoder encoder;
    int16_t duty;
    uint8_t started;
} DevMotor;

void Dev_Motor_Init(DevMotor *motor, const DevMotorConfig *config);
void Dev_Motor_Start(DevMotor *motor);
void Dev_Motor_SetDuty(DevMotor *motor, int16_t duty);
void Dev_Motor_SetSignedDuty(DevMotor *motor, int16_t duty);
void Dev_Motor_Coast(DevMotor *motor);
void Dev_Motor_Brake(DevMotor *motor);
int16_t Dev_Motor_GetDuty(const DevMotor *motor);

void Dev_Motor_PollEncoder(DevMotor *motor);
int32_t Dev_Motor_GetEncoderDelta(DevMotor *motor);
int64_t Dev_Motor_GetEncoderTotal(const DevMotor *motor);
void Dev_Motor_ResetEncoder(DevMotor *motor);

#endif
