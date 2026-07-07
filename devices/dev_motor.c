#include "dev_motor.h"
#include <string.h>

static int16_t max_duty(const DevMotor *motor)
{
    if ((motor == 0) || (motor->config.max_duty <= 0)) {
        return DEV_MOTOR_DUTY_MAX_DEFAULT;
    }

    return motor->config.max_duty;
}

static int16_t clamp_signed(const DevMotor *motor, int16_t duty)
{
    int16_t limit = max_duty(motor);

    if (duty > limit) {
        return limit;
    }
    if (duty < (int16_t)-limit) {
        return (int16_t)-limit;
    }
    return duty;
}

void Dev_Motor_Init(DevMotor *motor, const DevMotorConfig *config)
{
    if (motor == 0 || config == 0) {
        return;
    }

    memset(motor, 0, sizeof(*motor));
    motor->config = *config;
    if (motor->config.max_duty <= 0) {
        motor->config.max_duty = DEV_MOTOR_DUTY_MAX_DEFAULT;
    }
    if (motor->config.has_encoder) {
        Drv_QuadEncoder_Init(&motor->encoder, &motor->config.encoder);
    }
}

void Dev_Motor_Start(DevMotor *motor)
{
    if (motor == 0) {
        return;
    }

    motor->duty = 0;
    Drv_HBridgeMotor_Start(&motor->config.output);
    if (motor->config.has_encoder) {
        Drv_QuadEncoder_Reset(&motor->encoder);
    }
    motor->started = 1u;
}

void Dev_Motor_SetDuty(DevMotor *motor, int16_t duty)
{
    if (duty < 0) {
        duty = 0;
    }

    Dev_Motor_SetSignedDuty(motor, duty);
}

void Dev_Motor_SetSignedDuty(DevMotor *motor, int16_t duty)
{
    if (motor == 0) {
        return;
    }

    duty = clamp_signed(motor, duty);
    motor->duty = duty;
    if (duty == 0) {
        Drv_HBridgeMotor_Coast(&motor->config.output);
    } else {
        Drv_HBridgeMotor_SetSignedDuty(&motor->config.output, duty);
    }
}

void Dev_Motor_Coast(DevMotor *motor)
{
    if (motor == 0) {
        return;
    }

    motor->duty = 0;
    Drv_HBridgeMotor_Coast(&motor->config.output);
}

void Dev_Motor_Brake(DevMotor *motor)
{
    if (motor == 0) {
        return;
    }

    motor->duty = 0;
    Drv_HBridgeMotor_Brake(&motor->config.output);
}

int16_t Dev_Motor_GetDuty(const DevMotor *motor)
{
    return (motor != 0) ? motor->duty : 0;
}

void Dev_Motor_PollEncoder(DevMotor *motor)
{
    if ((motor == 0) || !motor->config.has_encoder) {
        return;
    }

    Drv_QuadEncoder_Poll(&motor->encoder);
}

int32_t Dev_Motor_GetEncoderDelta(DevMotor *motor)
{
    if ((motor == 0) || !motor->config.has_encoder) {
        return 0;
    }

    return Drv_QuadEncoder_GetDelta(&motor->encoder);
}

int64_t Dev_Motor_GetEncoderTotal(const DevMotor *motor)
{
    if ((motor == 0) || !motor->config.has_encoder) {
        return 0;
    }

    return Drv_QuadEncoder_GetTotal(&motor->encoder);
}

void Dev_Motor_ResetEncoder(DevMotor *motor)
{
    if ((motor == 0) || !motor->config.has_encoder) {
        return;
    }

    Drv_QuadEncoder_Reset(&motor->encoder);
}
