#ifndef PID_H
#define PID_H

typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_err;
    float last_setpoint;
    float last_measure;
    float last_error;
    float last_p;
    float last_i;
    float last_d;
    float last_deriv;
    float last_raw_output;
    float last_output;
    float out_min, out_max;
} PID_t;

void  PID_Init(PID_t *p, float kp, float ki, float kd, float omin, float omax);
float PID_Update(PID_t *p, float setpoint, float measure);
void  PID_SetGains(PID_t *p, float kp, float ki, float kd);
void  PID_Reset(PID_t *p);

#endif
