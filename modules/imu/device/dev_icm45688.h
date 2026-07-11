#ifndef DEV_ICM45688_H
#define DEV_ICM45688_H

#include <stdint.h>
#include "drv_spi.h"
#include "imu/inv_imu_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ICM45688_MSPM0_DEFAULT_ADDR        0x68u
#define ICM45688_MSPM0_DEFAULT_ACCEL_FSR_G 16.0f
#define ICM45688_MSPM0_DEFAULT_GYRO_FSR_DPS 1000.0f
#define ICM45688_MSPM0_DEFAULT_ODR_HZ      200u

typedef enum {
    ICM45688_MSPM0_ATTITUDE_COMPLEMENTARY = 0,
    ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS = 1
} icm45688_mspm0_attitude_mode_t;

typedef struct {
    DrvSpiBus bus;
    uint8_t ready;
    uint8_t last_who_am_i;
    uint8_t stationary;
    uint8_t gyro_bias_auto_update;
    uint8_t sample_lpf_initialized;
    uint16_t gyro_bias_stationary_samples;
    inv_imu_device_t tdk;

    float accel_fsr_g;
    float gyro_fsr_dps;
    float gyro_bias_x_dps;
    float gyro_bias_y_dps;
    float gyro_bias_z_dps;
    float gyro_scale_x;
    float gyro_scale_y;
    float gyro_scale_z;
    float stationary_gyro_threshold_dps;
    float stationary_accel_threshold_g;
    float gyro_bias_learning_rate;
    float accel_lpf_cutoff_hz;
    float gyro_lpf_cutoff_hz;
    float filter_dt_s;
    float filter_alpha;
    icm45688_mspm0_attitude_mode_t attitude_mode;
    float accel_trust_window_g;
    float accel_rejection_threshold_sq;
    float mahony_kp;
    float mahony_ki;
    float mahony_integral_x;
    float mahony_integral_y;
    float mahony_integral_z;
    float quat_w;
    float quat_x;
    float quat_y;
    float quat_z;
    uint16_t stationary_hold_samples;
    uint16_t stationary_sample_count;

    float ax_g;
    float ay_g;
    float az_g;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    float temp_c;
    float pitch_deg;
    float roll_deg;
    float yaw_deg;
} icm45688_mspm0_t;

void ICM45688_MSPM0_InitHandleSPI(icm45688_mspm0_t *dev,
                                  const DrvSpiBus *bus);
void ICM45688_MSPM0_SetFilter(icm45688_mspm0_t *dev, float dt_s, float alpha);
void ICM45688_MSPM0_SetSampleLowPass(icm45688_mspm0_t *dev,
                                     float accel_cutoff_hz,
                                     float gyro_cutoff_hz);
void ICM45688_MSPM0_ResetSampleLowPass(icm45688_mspm0_t *dev);
void ICM45688_MSPM0_SetAttitudeMode(icm45688_mspm0_t *dev,
                                    icm45688_mspm0_attitude_mode_t mode);
void ICM45688_MSPM0_SetAccelTrustWindow(icm45688_mspm0_t *dev,
                                        float max_accel_error_g);
void ICM45688_MSPM0_SetAccelRejection(icm45688_mspm0_t *dev,
                                      float rejection_angle_deg);
void ICM45688_MSPM0_SetMahonyGains(icm45688_mspm0_t *dev,
                                   float kp,
                                   float ki);
void ICM45688_MSPM0_ResetAttitude(icm45688_mspm0_t *dev);
void ICM45688_MSPM0_GetQuaternion(const icm45688_mspm0_t *dev,
                                  float *w,
                                  float *x,
                                  float *y,
                                  float *z);
void ICM45688_MSPM0_SetGyroBias(icm45688_mspm0_t *dev,
                                float gx_bias_dps,
                                float gy_bias_dps,
                                float gz_bias_dps);
void ICM45688_MSPM0_SetGyroScale(icm45688_mspm0_t *dev,
                                 float gx_scale,
                                 float gy_scale,
                                 float gz_scale);
void ICM45688_MSPM0_SetStationaryDetection(icm45688_mspm0_t *dev,
                                           float gyro_threshold_dps,
                                           float accel_threshold_g,
                                           uint16_t hold_samples);
void ICM45688_MSPM0_SetGyroBiasAutoUpdate(icm45688_mspm0_t *dev,
                                          uint8_t enable,
                                          float learning_rate);
int ICM45688_MSPM0_Begin(icm45688_mspm0_t *dev);
int ICM45688_MSPM0_ReadWhoAmI(icm45688_mspm0_t *dev, uint8_t *whoami);
int ICM45688_MSPM0_GetDataReady(icm45688_mspm0_t *dev, uint8_t *data_ready);
int ICM45688_MSPM0_Update(icm45688_mspm0_t *dev);
uint8_t ICM45688_MSPM0_IsReady(const icm45688_mspm0_t *dev);
uint8_t ICM45688_MSPM0_IsStationary(const icm45688_mspm0_t *dev);

#ifdef __cplusplus
}
#endif

#endif
