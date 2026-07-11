#ifndef APP_SHARED_H
#define APP_SHARED_H

#include "app_main.h"
#include "chassis.h"
#include "pid.h"
#include <stdint.h>

typedef struct {
    AppState_t state;
    uint8_t state_id;

    ControlChassis_t chassis;
    PID_t line_pid;
    float line_turn;

    float pitch_deg;
    float roll_deg;
    float yaw_deg;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    float ax_g;
    float ay_g;
    float az_g;
    uint8_t imu_ready;
} AppContext_t;

AppContext_t *App_Shared_Get(void);
void App_Shared_SetState(AppState_t state);

#endif /* APP_SHARED_H */
