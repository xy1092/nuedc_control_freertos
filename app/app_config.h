#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "vehicle_calibration.h"

/* Task periods. Keep fast sampling short; put blocking work in telemetry/tools. */
#define APP_FAST_TASK_PERIOD_MS       5u
#define APP_CONTROL_TASK_PERIOD_MS    10u
#define APP_TELEM_TASK_PERIOD_MS      10u

/* Chassis values verified on the assembled car. */
#define APP_LEFT_PULSES_PER_CM        VEHICLE_ENCODER_LEFT_PULSES_PER_CM
#define APP_RIGHT_PULSES_PER_CM       VEHICLE_ENCODER_RIGHT_PULSES_PER_CM
#define APP_WHEEL_BASE_CM             13.5f
#define APP_OPEN_LOOP_CM_PER_PWM_MS   0.00022f
#define APP_SPEED_OUT_LIMIT           1000.0f
#define APP_FORWARD_MIN_PULSE         0.03f

#define APP_SPEED_PID_KP              18.0f
#define APP_SPEED_PID_KI              0.0f
#define APP_SPEED_PID_KD              0.0f

#define APP_LINE_PID_KP               0.18f
#define APP_LINE_PID_KI               0.0f
#define APP_LINE_PID_KD               0.02f

/* Conservative default command for first bring-up. */
#define APP_BASE_SPEED_PULSE          80.0f
#define APP_LINE_TURN_LIMIT           40.0f

#endif /* APP_CONFIG_H */
