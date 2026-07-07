#include "bsp_motor.h"
#include "../pin_map.h"
#include "../drivers/drv_gpio.h"
#include "../devices/dev_motor.h"

#define PWM_MAX  1000

static DevMotor s_motor[2];

static const DevMotorConfig s_motor_config[2] = {
    {
        {
            { PWM_MOTOR_INST, PWM_CH_LEFT, PWM_MAX, DRV_PWM_ACTIVE_LOW },
            { MOTOR_DIR_PORT, LEFT_AIN1_PIN, 0u },
            { MOTOR_DIR_PORT, LEFT_AIN2_PIN, 0u },
            1,
        },
        {
            { ENC_LEFT_PORT, ENC_LEFT_A_PIN, 0u },
            { ENC_LEFT_PORT, ENC_LEFT_B_PIN, 0u },
            1,
        },
        PWM_MAX,
        1u,
    },
    {
        {
            { PWM_MOTOR_INST, PWM_CH_RIGHT, PWM_MAX, DRV_PWM_ACTIVE_LOW },
            { MOTOR_DIR_PORT, RIGHT_BIN1_PIN, 0u },
            { MOTOR_DIR_PORT, RIGHT_BIN2_PIN, 0u },
            1,
        },
        {
            { ENC_RIGHT_PORT, ENC_RIGHT_A_PIN, 0u },
            { ENC_RIGHT_PORT, ENC_RIGHT_B_PIN, 0u },
            1,
        },
        PWM_MAX,
        1u,
    },
};

#ifdef MOTOR_STBY_PIN
static const DrvGpioPin s_stby       = { MOTOR_DIR_PORT, MOTOR_STBY_PIN, 0u };
#endif

static inline uint8_t motor_index(uint8_t ch)
{
    return (ch < 2u) ? ch : 1u;
}

static inline void refresh_standby(void)
{
    BSP_Motor_Standby((Dev_Motor_GetDuty(&s_motor[0]) != 0 ||
                       Dev_Motor_GetDuty(&s_motor[1]) != 0) ? 1u : 0u);
}

void BSP_Motor_Init(void)
{
    Dev_Motor_Init(&s_motor[0], &s_motor_config[0]);
    Dev_Motor_Init(&s_motor[1], &s_motor_config[1]);
    Dev_Motor_Start(&s_motor[0]);
    Dev_Motor_Start(&s_motor[1]);
    BSP_Motor_Standby(0);
}

void BSP_Motor_SetDuty(uint8_t ch, int16_t duty)
{
    if (duty >  PWM_MAX) duty =  PWM_MAX;
    if (duty < 0) duty = 0;

    BSP_Motor_SetSignedDuty(ch, duty);
}

void BSP_Motor_SetSignedDuty(uint8_t ch, int16_t duty)
{
    uint8_t idx = motor_index(ch);

    if (duty >  PWM_MAX) duty =  PWM_MAX;
    if (duty < -PWM_MAX) duty = -PWM_MAX;

    Dev_Motor_SetSignedDuty(&s_motor[idx], duty);
    refresh_standby();
}

void BSP_Motor_Brake(uint8_t ch)
{
    uint8_t idx = motor_index(ch);

    Dev_Motor_Coast(&s_motor[idx]);
    refresh_standby();
}

void BSP_Motor_Standby(uint8_t on)
{
#ifdef MOTOR_STBY_PIN
    Drv_Gpio_WriteRaw(&s_stby, on ? 1u : 0u);
#else
    (void)on;
#endif
}

DevMotor *BSP_Motor_GetDevice(uint8_t ch)
{
    return &s_motor[motor_index(ch)];
}
