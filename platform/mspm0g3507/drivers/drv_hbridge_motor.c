#include "drv_hbridge_motor.h"

#define HBRIDGE_DUTY_MAX 1000

static int16_t clamp_duty(int16_t duty)
{
    if (duty > HBRIDGE_DUTY_MAX) {
        return HBRIDGE_DUTY_MAX;
    }
    if (duty < -HBRIDGE_DUTY_MAX) {
        return -HBRIDGE_DUTY_MAX;
    }
    return duty;
}

void Drv_HBridgeMotor_Start(const DrvHBridgeMotor *motor)
{
    if (motor == 0) {
        return;
    }

    Drv_Pwm_SetDutyPermille(&motor->pwm, 0u);
    Drv_HBridgeMotor_Coast(motor);
    Drv_Pwm_Start(&motor->pwm);
}

void Drv_HBridgeMotor_Coast(const DrvHBridgeMotor *motor)
{
    if (motor == 0) {
        return;
    }

    Drv_Pwm_SetDutyPermille(&motor->pwm, 0u);
    Drv_Gpio_WriteRaw(&motor->in1, 0u);
    Drv_Gpio_WriteRaw(&motor->in2, 0u);
}

void Drv_HBridgeMotor_Brake(const DrvHBridgeMotor *motor)
{
    if (motor == 0) {
        return;
    }

    Drv_Pwm_SetDutyPermille(&motor->pwm, 0u);
    Drv_Gpio_WriteRaw(&motor->in1, 1u);
    Drv_Gpio_WriteRaw(&motor->in2, 1u);
}

void Drv_HBridgeMotor_SetSignedDuty(const DrvHBridgeMotor *motor, int16_t duty)
{
    uint16_t magnitude;
    int8_t direction;

    if (motor == 0) {
        return;
    }

    duty = clamp_duty(duty);
    if (duty == 0) {
        Drv_HBridgeMotor_Coast(motor);
        return;
    }

    direction = (motor->direction == 0) ? 1 : motor->direction;
    if (direction < 0) {
        duty = (int16_t)(-duty);
    }

    if (duty > 0) {
        Drv_Gpio_WriteRaw(&motor->in1, 0u);
        Drv_Gpio_WriteRaw(&motor->in2, 1u);
        magnitude = (uint16_t)duty;
    } else {
        Drv_Gpio_WriteRaw(&motor->in1, 1u);
        Drv_Gpio_WriteRaw(&motor->in2, 0u);
        magnitude = (uint16_t)(-duty);
    }

    Drv_Pwm_SetDutyPermille(&motor->pwm, magnitude);
}
