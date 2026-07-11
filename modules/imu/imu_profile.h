#ifndef IMU_PROFILE_H
#define IMU_PROFILE_H

#include <stdint.h>

typedef struct {
    float sample_period_s;
    float filter_alpha;
    float accel_lpf_hz;
    float gyro_lpf_hz;
    float stationary_gyro_dps;
    float stationary_accel_g;
    uint16_t stationary_hold_samples;
    float accel_trust_window_g;
    float accel_rejection_deg;
    float mahony_kp;
    float mahony_ki;
    uint16_t gyro_cal_warmup_samples;
    uint16_t gyro_cal_samples;
    float gyro_cal_stddev_limit_dps;
    float gyro_bias_learning_rate;
    float gyro_z_scale;
} ImuCalibratedProfile_t;

extern const ImuCalibratedProfile_t g_imu_calibrated_profile;

#endif
