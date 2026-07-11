#ifndef MIDDLEWARE_CONTROL_CHASSIS_H
#define MIDDLEWARE_CONTROL_CHASSIS_H

#include <stdint.h>
#include "pid.h"

typedef struct {
    float x;
    float y;
    float theta;
} ControlPose2D_t;

typedef struct {
    float left_pulses_per_cm;
    float right_pulses_per_cm;
    float wheel_base_cm;
    float open_loop_cm_per_pwm_ms;
    float speed_out_limit;
    float forward_min_pulse;
} ControlChassisConfig_t;

typedef struct {
    ControlChassisConfig_t config;
    PID_t pid_left;
    PID_t pid_right;

    ControlPose2D_t pose;
    float target_speed_left;
    float target_speed_right;
    float measured_speed_left;
    float measured_speed_right;
    float segment_dist_cm;
    float dead_zone_left;
    float dead_zone_right;

    uint8_t debug_pwm_active;
    int16_t debug_pwm_left;
    int16_t debug_pwm_right;
    uint32_t debug_pwm_deadline_ms;
} ControlChassis_t;

void Control_Chassis_Init(ControlChassis_t *chassis,
                          const ControlChassisConfig_t *config,
                          float speed_kp, float speed_ki, float speed_kd);
void Control_Chassis_PollFastInputs(ControlChassis_t *chassis);
void Control_Chassis_ResetOdometry(ControlChassis_t *chassis, ControlPose2D_t pose);
void Control_Chassis_ResetSegmentDistance(ControlChassis_t *chassis);
void Control_Chassis_SetTargetSpeeds(ControlChassis_t *chassis, float left, float right);
void Control_Chassis_SetDeadZone(ControlChassis_t *chassis, float left, float right);
void Control_Chassis_Tick(ControlChassis_t *chassis, uint8_t run, uint32_t now_ms);
void Control_Chassis_SetDebugDuty(ControlChassis_t *chassis,
                                  int16_t left_pwm, int16_t right_pwm,
                                  uint32_t now_ms, uint32_t hold_ms);
void Control_Chassis_ClearDebugDuty(ControlChassis_t *chassis);

#endif
