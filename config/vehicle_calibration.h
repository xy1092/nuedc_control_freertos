#ifndef VEHICLE_CALIBRATION_H
#define VEHICLE_CALIBRATION_H

/* Physical calibration verified on the assembled car on 2026-07-11. */
#define VEHICLE_ENCODER_LEFT_PULSES_PER_CM       73.44f
#define VEHICLE_ENCODER_RIGHT_PULSES_PER_CM      73.14f

/* Straight-line calibration at nominal battery voltage. */
#define VEHICLE_STRAIGHT_PWM_LEFT                300
#define VEHICLE_STRAIGHT_PWM_RIGHT               300
#define VEHICLE_STRAIGHT_SYNC_KP                 0.08f
#define VEHICLE_STRAIGHT_SYNC_DEADBAND_PULSES    3
#define VEHICLE_STRAIGHT_SYNC_MAX_PWM            15
#define VEHICLE_STRAIGHT_DECEL_WINDOW_PULSES     900u
#define VEHICLE_STRAIGHT_MIN_PWM                 220
#define VEHICLE_STRAIGHT_RAMP_MS                 500u

/* Stationary 90-degree yaw turn calibration. */
#define VEHICLE_TURN_ANGLE_DEG                    90.0f
#define VEHICLE_TURN_PRECISION_DEG                0.2f
#define VEHICLE_TURN_TIMEOUT_MS                   12000u
#define VEHICLE_TURN_FINE_MAX_ATTEMPTS            20u
#define VEHICLE_TURN_PWM_MAX                      300
#define VEHICLE_TURN_PWM_MIN                      220
#define VEHICLE_TURN_KP                           4.0f
#define VEHICLE_TURN_APPROACH_WINDOW_DEG          20.0f
#define VEHICLE_TURN_PWM_APPROACH_MIN             160
#define VEHICLE_TURN_FINE_ENTRY_DEG               3.0f
#define VEHICLE_TURN_SETTLE_RATE_DPS              1.0f
#define VEHICLE_TURN_SETTLE_SAMPLES               15u
#define VEHICLE_TURN_BRAKE_MIN_MS                 120u
#define VEHICLE_TURN_FINE_PWM_INITIAL             180
#define VEHICLE_TURN_FINE_PWM_MIN                 140
#define VEHICLE_TURN_FINE_PWM_MAX                 260
#define VEHICLE_TURN_FINE_PWM_STEP                20
#define VEHICLE_TURN_FINE_PULSE_MS                20u
#define VEHICLE_TURN_FINE_PROGRESS_MIN_DEG        0.05f
#define VEHICLE_TURN_DIRECTION_CHECK_MS           600u
#define VEHICLE_TURN_DIRECTION_PROGRESS_DEG       2.0f

#endif /* VEHICLE_CALIBRATION_H */
