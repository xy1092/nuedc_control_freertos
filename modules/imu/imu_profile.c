#include "imu_profile.h"

/* Values copied from mspm0-bmi088-icm45688 Sources/main.c. */
const ImuCalibratedProfile_t g_imu_calibrated_profile = {
    .sample_period_s = 0.005f,
    .filter_alpha = 0.98f,
    .accel_lpf_hz = 30.0f,
    .gyro_lpf_hz = 0.0f,
    .stationary_gyro_dps = 1.5f,
    .stationary_accel_g = 0.08f,
    .stationary_hold_samples = 5u,
    .accel_trust_window_g = 0.15f,
    .accel_rejection_deg = 10.0f,
    .mahony_kp = 1.0f,
    .mahony_ki = 0.0f,
    .gyro_cal_warmup_samples = 200u,
    .gyro_cal_samples = 2000u,
    .gyro_cal_stddev_limit_dps = 0.50f,
    .gyro_bias_learning_rate = 0.002f,
    .gyro_z_scale = 0.994821f,
};
