#ifndef DRV_HBRIDGE_MOTOR_H
#define DRV_HBRIDGE_MOTOR_H

#include <stdint.h>
#include "drv_gpio.h"
#include "drv_pwm.h"

typedef struct {
    DrvPwmChannel pwm;
    DrvGpioPin in1;
    DrvGpioPin in2;
    int8_t direction;
} DrvHBridgeMotor;

void Drv_HBridgeMotor_Start(const DrvHBridgeMotor *motor);
void Drv_HBridgeMotor_Coast(const DrvHBridgeMotor *motor);
void Drv_HBridgeMotor_Brake(const DrvHBridgeMotor *motor);
void Drv_HBridgeMotor_SetSignedDuty(const DrvHBridgeMotor *motor, int16_t duty);

#endif
