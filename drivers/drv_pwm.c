#include "drv_pwm.h"

#define DRV_PWM_DUTY_MAX 1000u

static uint32_t clamp_u32(uint32_t value, uint32_t max_value)
{
    return (value > max_value) ? max_value : value;
}

void Drv_Pwm_Start(const DrvPwmChannel *pwm)
{
    if ((pwm == 0) || (pwm->timer == 0)) {
        return;
    }

    DL_Timer_startCounter(pwm->timer);
}

void Drv_Pwm_SetCompareRaw(const DrvPwmChannel *pwm, uint32_t compare)
{
    if ((pwm == 0) || (pwm->timer == 0)) {
        return;
    }

    DL_Timer_setCaptureCompareValue(pwm->timer, compare, pwm->channel);
}

void Drv_Pwm_SetDutyPermille(const DrvPwmChannel *pwm, uint16_t duty)
{
    uint32_t duty_clamped;
    uint32_t compare;

    if ((pwm == 0) || (pwm->timer == 0)) {
        return;
    }

    duty_clamped = clamp_u32(duty, DRV_PWM_DUTY_MAX);
    compare = (pwm->period_ticks * duty_clamped) / DRV_PWM_DUTY_MAX;

    if (pwm->polarity == DRV_PWM_ACTIVE_LOW) {
        compare = pwm->period_ticks - compare;
    }

    Drv_Pwm_SetCompareRaw(pwm, compare);
}
