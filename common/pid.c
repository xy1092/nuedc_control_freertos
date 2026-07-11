#include "pid.h"
#include <math.h>

void PID_Init(PID_t *p, float kp, float ki, float kd, float omin, float omax)
{
    p->kp = kp; p->ki = ki; p->kd = kd;
    p->integral = 0.0f;
    p->prev_err = 0.0f;
    p->last_setpoint = 0.0f;
    p->last_measure = 0.0f;
    p->last_error = 0.0f;
    p->last_p = 0.0f;
    p->last_i = 0.0f;
    p->last_d = 0.0f;
    p->last_deriv = 0.0f;
    p->last_raw_output = 0.0f;
    p->last_output = 0.0f;
    p->out_min  = omin;
    p->out_max  = omax;
}

float PID_Update(PID_t *p, float setpoint, float measure)
{
    float err = setpoint - measure;
    float deriv = err - p->prev_err;
    float out_span;
    float i_limit;
    float out;

    p->integral += err;

    /* 积分限幅：钳制到 ki 贡献不超过输出量程 */
    if (fabsf(p->ki) > 1e-8f) {
        out_span = fmaxf(fabsf(p->out_min), fabsf(p->out_max));
        i_limit = out_span / fabsf(p->ki);
        if (p->integral >  i_limit) p->integral =  i_limit;
        if (p->integral < -i_limit) p->integral = -i_limit;
    }

    p->last_setpoint = setpoint;
    p->last_measure = measure;
    p->last_error = err;
    p->last_p = p->kp * err;
    p->last_i = p->ki * p->integral;
    p->last_d = p->kd * deriv;
    p->last_deriv = deriv;
    out = p->last_p + p->last_i + p->last_d;
    p->last_raw_output = out;

    p->prev_err = err;

    if (out > p->out_max) out = p->out_max;
    if (out < p->out_min) out = p->out_min;
    p->last_output = out;
    return out;
}

void PID_SetGains(PID_t *p, float kp, float ki, float kd)
{
    p->kp = kp; p->ki = ki; p->kd = kd;
}

void PID_Reset(PID_t *p)
{
    p->integral = 0.0f;
    p->prev_err = 0.0f;
    p->last_setpoint = 0.0f;
    p->last_measure = 0.0f;
    p->last_error = 0.0f;
    p->last_p = 0.0f;
    p->last_i = 0.0f;
    p->last_d = 0.0f;
    p->last_deriv = 0.0f;
    p->last_raw_output = 0.0f;
    p->last_output = 0.0f;
}
