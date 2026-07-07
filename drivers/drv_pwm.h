#ifndef DRV_PWM_H
#define DRV_PWM_H

#include <stdint.h>
#include <ti/driverlib/driverlib.h>

typedef enum {
    DRV_PWM_ACTIVE_HIGH = 0,
    DRV_PWM_ACTIVE_LOW = 1,
} DrvPwmPolarity;

typedef struct {
    GPTIMER_Regs *timer;
    DL_TIMER_CC_INDEX channel;
    uint32_t period_ticks;
    DrvPwmPolarity polarity;
} DrvPwmChannel;

void Drv_Pwm_Start(const DrvPwmChannel *pwm);
void Drv_Pwm_SetCompareRaw(const DrvPwmChannel *pwm, uint32_t compare);
void Drv_Pwm_SetDutyPermille(const DrvPwmChannel *pwm, uint16_t duty);

#endif
