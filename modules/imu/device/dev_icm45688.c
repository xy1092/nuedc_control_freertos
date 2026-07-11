#include "dev_icm45688.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifndef ICM45688_MSPM0_CPU_HZ
#ifdef CPUCLK_FREQ
#define ICM45688_MSPM0_CPU_HZ CPUCLK_FREQ
#else
#define ICM45688_MSPM0_CPU_HZ 32000000u
#endif
#endif

#define ICM45688_RAW_MAX           32768.0f
#define ICM45688_DEG_TO_RAD        0.0174532925f
#define ICM45688_RAD_TO_DEG        57.2957795f
#define ICM45688_DEFAULT_MAHONY_KP 2.0f
#define ICM45688_DEFAULT_MAHONY_KI 0.0f
#define ICM45688_MIN_NORM_SQ       0.000001f
#define ICM45688_DEFAULT_STATIONARY_GYRO_DPS 1.5f
#define ICM45688_DEFAULT_STATIONARY_ACCEL_G  0.08f
#define ICM45688_DEFAULT_STATIONARY_HOLD     5u
#define ICM45688_DEFAULT_BIAS_LEARN_RATE     0.0010f
#define ICM45688_TWO_PI                      6.283185307f
#define ICM45688_YAW_LOCK_GYRO_DPS           0.25f
#define ICM45688_BIAS_LEARN_STATIONARY_SAMPLES 600u
#define ICM45688_BIAS_LEARN_GYRO_DPS         0.35f

static void icm_sleep_us(uint32_t us)
{
    uint32_t cycles_per_us = ICM45688_MSPM0_CPU_HZ / 1000000u;
    if (cycles_per_us == 0u) {
        cycles_per_us = 1u;
    }

    while (us > 1000u) {
        delay_cycles(cycles_per_us * 1000u);
        us -= 1000u;
    }
    if (us > 0u) {
        delay_cycles(cycles_per_us * us);
    }
}

static int icm_spi_write(void *context, uint8_t reg, const uint8_t *buf, uint32_t len)
{
    icm45688_mspm0_t *dev = (icm45688_mspm0_t *)context;

    if (dev == NULL || buf == NULL || len == 0u) {
        return -1;
    }

    return Drv_Spi_WriteReg8(&dev->bus, reg, buf, len);
}

static int icm_spi_read(void *context, uint8_t reg, uint8_t *buf, uint32_t len)
{
    icm45688_mspm0_t *dev = (icm45688_mspm0_t *)context;
    if (dev == NULL || dev->bus.inst == NULL || buf == NULL || len == 0u) {
        return -1;
    }

    return Drv_Spi_ReadReg8(&dev->bus, reg, buf, len);
}

static void clear_measurements(icm45688_mspm0_t *dev)
{
    dev->ax_g = 0.0f;
    dev->ay_g = 0.0f;
    dev->az_g = 0.0f;
    dev->gx_dps = 0.0f;
    dev->gy_dps = 0.0f;
    dev->gz_dps = 0.0f;
    dev->temp_c = 0.0f;
    dev->pitch_deg = 0.0f;
    dev->roll_deg = 0.0f;
    dev->yaw_deg = 0.0f;
    dev->stationary = 0u;
    dev->stationary_sample_count = 0u;
    dev->gyro_bias_stationary_samples = 0u;
    dev->sample_lpf_initialized = 0u;
    ICM45688_MSPM0_ResetAttitude(dev);
}

static void init_attitude_defaults(icm45688_mspm0_t *dev)
{
    dev->filter_dt_s = 1.0f / (float)ICM45688_MSPM0_DEFAULT_ODR_HZ;
    dev->filter_alpha = 0.98f;
    dev->attitude_mode = ICM45688_MSPM0_ATTITUDE_COMPLEMENTARY;
    dev->stationary_gyro_threshold_dps = ICM45688_DEFAULT_STATIONARY_GYRO_DPS;
    dev->stationary_accel_threshold_g = ICM45688_DEFAULT_STATIONARY_ACCEL_G;
    dev->stationary_hold_samples = ICM45688_DEFAULT_STATIONARY_HOLD;
    dev->gyro_bias_auto_update = 0u;
    dev->gyro_bias_learning_rate = ICM45688_DEFAULT_BIAS_LEARN_RATE;
    dev->gyro_scale_x = 1.0f;
    dev->gyro_scale_y = 1.0f;
    dev->gyro_scale_z = 1.0f;
    dev->accel_lpf_cutoff_hz = 0.0f;
    dev->gyro_lpf_cutoff_hz = 0.0f;
    dev->accel_trust_window_g = 0.0f;
    dev->accel_rejection_threshold_sq = 0.0f;
    dev->mahony_kp = ICM45688_DEFAULT_MAHONY_KP;
    dev->mahony_ki = ICM45688_DEFAULT_MAHONY_KI;
    ICM45688_MSPM0_ResetAttitude(dev);
}

static float clampf_local(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static float lpf_alpha(float cutoff_hz, float dt_s)
{
    float x;

    if (cutoff_hz <= 0.0f || dt_s <= 0.0f) {
        return 1.0f;
    }

    x = ICM45688_TWO_PI * cutoff_hz * dt_s;
    return clampf_local(x / (1.0f + x), 0.0f, 1.0f);
}

static float lpf_step(float prev, float input, float cutoff_hz, float dt_s)
{
    float alpha = lpf_alpha(cutoff_hz, dt_s);
    return prev + alpha * (input - prev);
}

static uint8_t yaw_should_lock(const icm45688_mspm0_t *dev)
{
    return (dev != NULL &&
            dev->stationary &&
            fabsf(dev->gz_dps) <= ICM45688_YAW_LOCK_GYRO_DPS) ? 1u : 0u;
}

static float wrap_degrees_180(float angle_deg)
{
    while (angle_deg > 180.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg <= -180.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

static void apply_sample_lowpass(icm45688_mspm0_t *dev,
                                 float raw_ax_g,
                                 float raw_ay_g,
                                 float raw_az_g,
                                 float corrected_gx_dps,
                                 float corrected_gy_dps,
                                 float corrected_gz_dps)
{
    if (dev == NULL) {
        return;
    }

    if (!dev->sample_lpf_initialized) {
        dev->ax_g = raw_ax_g;
        dev->ay_g = raw_ay_g;
        dev->az_g = raw_az_g;
        dev->gx_dps = corrected_gx_dps;
        dev->gy_dps = corrected_gy_dps;
        dev->gz_dps = corrected_gz_dps;
        dev->sample_lpf_initialized = 1u;
        return;
    }

    dev->ax_g = lpf_step(dev->ax_g, raw_ax_g,
                         dev->accel_lpf_cutoff_hz, dev->filter_dt_s);
    dev->ay_g = lpf_step(dev->ay_g, raw_ay_g,
                         dev->accel_lpf_cutoff_hz, dev->filter_dt_s);
    dev->az_g = lpf_step(dev->az_g, raw_az_g,
                         dev->accel_lpf_cutoff_hz, dev->filter_dt_s);
    dev->gx_dps = lpf_step(dev->gx_dps, corrected_gx_dps,
                           dev->gyro_lpf_cutoff_hz, dev->filter_dt_s);
    dev->gy_dps = lpf_step(dev->gy_dps, corrected_gy_dps,
                           dev->gyro_lpf_cutoff_hz, dev->filter_dt_s);
    dev->gz_dps = lpf_step(dev->gz_dps, corrected_gz_dps,
                           dev->gyro_lpf_cutoff_hz, dev->filter_dt_s);
}

static uint8_t accel_is_trusted(const icm45688_mspm0_t *dev, float *accel_norm)
{
    float norm_sq;
    float norm;

    if (dev == NULL) {
        return 0u;
    }

    norm_sq = dev->ax_g * dev->ax_g +
              dev->ay_g * dev->ay_g +
              dev->az_g * dev->az_g;
    if (norm_sq <= ICM45688_MIN_NORM_SQ) {
        return 0u;
    }

    if (dev->accel_trust_window_g <= 0.0f) {
        if (accel_norm != NULL) {
            *accel_norm = sqrtf(norm_sq);
        }
        return 1u;
    }

    norm = sqrtf(norm_sq);
    if (accel_norm != NULL) {
        *accel_norm = norm;
    }

    return (fabsf(norm - 1.0f) <= dev->accel_trust_window_g) ? 1u : 0u;
}

static void quat_from_euler(icm45688_mspm0_t *dev)
{
    float half_roll = dev->pitch_deg * ICM45688_DEG_TO_RAD * 0.5f;
    float half_pitch = dev->roll_deg * ICM45688_DEG_TO_RAD * 0.5f;
    float half_yaw = dev->yaw_deg * ICM45688_DEG_TO_RAD * 0.5f;
    float cr = cosf(half_roll);
    float sr = sinf(half_roll);
    float cp = cosf(half_pitch);
    float sp = sinf(half_pitch);
    float cy = cosf(half_yaw);
    float sy = sinf(half_yaw);

    dev->quat_w = cr * cp * cy + sr * sp * sy;
    dev->quat_x = sr * cp * cy - cr * sp * sy;
    dev->quat_y = cr * sp * cy + sr * cp * sy;
    dev->quat_z = cr * cp * sy - sr * sp * cy;
}

static void euler_from_quat(icm45688_mspm0_t *dev)
{
    float qw = dev->quat_w;
    float qx = dev->quat_x;
    float qy = dev->quat_y;
    float qz = dev->quat_z;
    float sin_pitch_y;
    float previous_yaw_deg = dev->yaw_deg;
    float yaw_wrapped_deg;
    float yaw_delta_deg;

    dev->pitch_deg = atan2f(2.0f * (qw * qx + qy * qz),
                            1.0f - 2.0f * (qx * qx + qy * qy)) *
                     ICM45688_RAD_TO_DEG;
    sin_pitch_y = 2.0f * (qw * qy - qz * qx);
    dev->roll_deg = asinf(clampf_local(sin_pitch_y, -1.0f, 1.0f)) *
                    ICM45688_RAD_TO_DEG;
    yaw_wrapped_deg = atan2f(2.0f * (qw * qz + qx * qy),
                             1.0f - 2.0f * (qy * qy + qz * qz)) *
                      ICM45688_RAD_TO_DEG;
    yaw_delta_deg = wrap_degrees_180(yaw_wrapped_deg -
                                     wrap_degrees_180(previous_yaw_deg));
    dev->yaw_deg = previous_yaw_deg + yaw_delta_deg;
}

static void update_complementary_attitude(icm45688_mspm0_t *dev)
{
    float alpha = dev->filter_alpha;
    float gyro_pitch = dev->pitch_deg + dev->gx_dps * dev->filter_dt_s;
    float gyro_roll = dev->roll_deg + dev->gy_dps * dev->filter_dt_s;

    if (accel_is_trusted(dev, NULL)) {
        float acc_pitch = atan2f(dev->ay_g, dev->az_g) * ICM45688_RAD_TO_DEG;
        float acc_roll = atan2f(-dev->ax_g,
                                sqrtf(dev->ay_g * dev->ay_g +
                                      dev->az_g * dev->az_g)) *
                         ICM45688_RAD_TO_DEG;

        dev->pitch_deg = alpha * gyro_pitch + (1.0f - alpha) * acc_pitch;
        dev->roll_deg = alpha * gyro_roll + (1.0f - alpha) * acc_roll;
    } else {
        dev->pitch_deg = gyro_pitch;
        dev->roll_deg = gyro_roll;
    }

    if (!yaw_should_lock(dev)) {
        dev->yaw_deg += dev->gz_dps * dev->filter_dt_s;
    }
}

static void update_mahony_attitude(icm45688_mspm0_t *dev)
{
    float accel_norm = 0.0f;
    float gx = dev->gx_dps * ICM45688_DEG_TO_RAD;
    float gy = dev->gy_dps * ICM45688_DEG_TO_RAD;
    float gz = dev->gz_dps * ICM45688_DEG_TO_RAD;
    float held_yaw_deg = dev->yaw_deg;
    uint8_t yaw_locked = yaw_should_lock(dev);

    if (accel_is_trusted(dev, &accel_norm)) {
        float ax = dev->ax_g / accel_norm;
        float ay = dev->ay_g / accel_norm;
        float az = dev->az_g / accel_norm;
        float halfvx = dev->quat_x * dev->quat_z - dev->quat_w * dev->quat_y;
        float halfvy = dev->quat_w * dev->quat_x + dev->quat_y * dev->quat_z;
        float halfvz = dev->quat_w * dev->quat_w - 0.5f +
                       dev->quat_z * dev->quat_z;
        float halfex = ay * halfvz - az * halfvy;
        float halfey = az * halfvx - ax * halfvz;
        float halfez = ax * halfvy - ay * halfvx;
        float feedback_norm_sq = halfex * halfex +
                                 halfey * halfey +
                                 halfez * halfez;
        uint8_t feedback_trusted =
            (dev->stationary ||
             dev->accel_rejection_threshold_sq <= 0.0f ||
             feedback_norm_sq <= dev->accel_rejection_threshold_sq) ? 1u : 0u;

        if (feedback_trusted && dev->mahony_ki > 0.0f) {
            dev->mahony_integral_x += dev->mahony_ki * halfex * dev->filter_dt_s;
            dev->mahony_integral_y += dev->mahony_ki * halfey * dev->filter_dt_s;
            dev->mahony_integral_z += dev->mahony_ki * halfez * dev->filter_dt_s;
            gx += dev->mahony_integral_x;
            gy += dev->mahony_integral_y;
            gz += dev->mahony_integral_z;
        } else if (dev->mahony_ki <= 0.0f) {
            dev->mahony_integral_x = 0.0f;
            dev->mahony_integral_y = 0.0f;
            dev->mahony_integral_z = 0.0f;
        }

        if (feedback_trusted) {
            gx += dev->mahony_kp * halfex;
            gy += dev->mahony_kp * halfey;
            gz += dev->mahony_kp * halfez;
        }
    }

    if (yaw_locked) {
        gz = 0.0f;
        dev->mahony_integral_z = 0.0f;
    }

    float qw = dev->quat_w;
    float qx = dev->quat_x;
    float qy = dev->quat_y;
    float qz = dev->quat_z;
    float half_dt = 0.5f * dev->filter_dt_s;
    float half_angle_sq = (gx * gx + gy * gy + gz * gz) *
                          half_dt * half_dt;
    float half_angle_pow4 = half_angle_sq * half_angle_sq;
    float delta_w = 1.0f - 0.5f * half_angle_sq +
                    half_angle_pow4 * (1.0f / 24.0f);
    float vector_scale = half_dt *
                         (1.0f - half_angle_sq * (1.0f / 6.0f) +
                          half_angle_pow4 * (1.0f / 120.0f));
    float delta_x = gx * vector_scale;
    float delta_y = gy * vector_scale;
    float delta_z = gz * vector_scale;

    dev->quat_w = qw * delta_w - qx * delta_x -
                  qy * delta_y - qz * delta_z;
    dev->quat_x = qw * delta_x + qx * delta_w +
                  qy * delta_z - qz * delta_y;
    dev->quat_y = qw * delta_y - qx * delta_z +
                  qy * delta_w + qz * delta_x;
    dev->quat_z = qw * delta_z + qx * delta_y -
                  qy * delta_x + qz * delta_w;

    float norm_sq = dev->quat_w * dev->quat_w +
                    dev->quat_x * dev->quat_x +
                    dev->quat_y * dev->quat_y +
                    dev->quat_z * dev->quat_z;
    if (norm_sq > ICM45688_MIN_NORM_SQ) {
        float inv_norm = 1.0f / sqrtf(norm_sq);
        dev->quat_w *= inv_norm;
        dev->quat_x *= inv_norm;
        dev->quat_y *= inv_norm;
        dev->quat_z *= inv_norm;
    } else {
        ICM45688_MSPM0_ResetAttitude(dev);
    }

    euler_from_quat(dev);
    if (yaw_locked) {
        dev->yaw_deg = held_yaw_deg;
        quat_from_euler(dev);
    }
}

static void update_stationary_state(icm45688_mspm0_t *dev)
{
    float accel_norm_sq;
    float accel_err_g;
    float gyro_mag_dps;
    uint8_t instant_stationary = 0u;
    uint16_t hold_samples;

    if (dev == NULL) {
        return;
    }

    accel_norm_sq = dev->ax_g * dev->ax_g +
                    dev->ay_g * dev->ay_g +
                    dev->az_g * dev->az_g;
    if (accel_norm_sq > ICM45688_MIN_NORM_SQ &&
        dev->stationary_gyro_threshold_dps > 0.0f &&
        dev->stationary_accel_threshold_g >= 0.0f) {
        gyro_mag_dps = sqrtf(dev->gx_dps * dev->gx_dps +
                             dev->gy_dps * dev->gy_dps +
                             dev->gz_dps * dev->gz_dps);
        accel_err_g = fabsf(sqrtf(accel_norm_sq) - 1.0f);
        if (gyro_mag_dps <= dev->stationary_gyro_threshold_dps &&
            accel_err_g <= dev->stationary_accel_threshold_g) {
            instant_stationary = 1u;
        }
    }

    hold_samples = (dev->stationary_hold_samples == 0u) ?
                   1u : dev->stationary_hold_samples;
    if (instant_stationary) {
        if (dev->stationary_sample_count < hold_samples) {
            dev->stationary_sample_count++;
        }
    } else {
        dev->stationary_sample_count = 0u;
    }

    dev->stationary =
        (dev->stationary_sample_count >= hold_samples) ? 1u : 0u;
}

static void update_auto_gyro_bias(icm45688_mspm0_t *dev,
                                  float raw_gx_dps,
                                  float raw_gy_dps,
                                  float raw_gz_dps)
{
    float learn;

    if (dev == NULL || !dev->gyro_bias_auto_update) {
        return;
    }

    if (!dev->stationary ||
        fabsf(dev->gx_dps) > ICM45688_BIAS_LEARN_GYRO_DPS ||
        fabsf(dev->gy_dps) > ICM45688_BIAS_LEARN_GYRO_DPS ||
        fabsf(dev->gz_dps) > ICM45688_BIAS_LEARN_GYRO_DPS) {
        dev->gyro_bias_stationary_samples = 0u;
        return;
    }

    if (dev->gyro_bias_stationary_samples < ICM45688_BIAS_LEARN_STATIONARY_SAMPLES) {
        dev->gyro_bias_stationary_samples++;
        return;
    }

    learn = clampf_local(dev->gyro_bias_learning_rate, 0.0f, 1.0f);
    if (learn <= 0.0f) {
        return;
    }

    dev->gyro_bias_x_dps += (raw_gx_dps - dev->gyro_bias_x_dps) * learn;
    dev->gyro_bias_y_dps += (raw_gy_dps - dev->gyro_bias_y_dps) * learn;
    dev->gyro_bias_z_dps += (raw_gz_dps - dev->gyro_bias_z_dps) * learn;
}

void ICM45688_MSPM0_InitHandleSPI(icm45688_mspm0_t *dev,
                                  const DrvSpiBus *bus)
{
    if (dev == NULL) {
        return;
    }

    memset(dev, 0, sizeof(*dev));
    if (bus != NULL) {
        dev->bus = *bus;
    }
    dev->accel_fsr_g = ICM45688_MSPM0_DEFAULT_ACCEL_FSR_G;
    dev->gyro_fsr_dps = ICM45688_MSPM0_DEFAULT_GYRO_FSR_DPS;
    init_attitude_defaults(dev);

    dev->tdk.transport.context = dev;
    Drv_Spi_Deselect(&dev->bus);
    dev->tdk.transport.read_reg = icm_spi_read;
    dev->tdk.transport.write_reg = icm_spi_write;
    dev->tdk.transport.serif_type = UI_SPI4;
    dev->tdk.transport.sleep_us = icm_sleep_us;
}

void ICM45688_MSPM0_SetFilter(icm45688_mspm0_t *dev, float dt_s, float alpha)
{
    if (dev == NULL) {
        return;
    }
    if (dt_s > 0.0f) {
        dev->filter_dt_s = dt_s;
    }
    if (alpha >= 0.0f && alpha <= 1.0f) {
        dev->filter_alpha = alpha;
    }
}

void ICM45688_MSPM0_SetSampleLowPass(icm45688_mspm0_t *dev,
                                     float accel_cutoff_hz,
                                     float gyro_cutoff_hz)
{
    if (dev == NULL) {
        return;
    }

    dev->accel_lpf_cutoff_hz =
        (accel_cutoff_hz > 0.0f) ? accel_cutoff_hz : 0.0f;
    dev->gyro_lpf_cutoff_hz =
        (gyro_cutoff_hz > 0.0f) ? gyro_cutoff_hz : 0.0f;
}

void ICM45688_MSPM0_ResetSampleLowPass(icm45688_mspm0_t *dev)
{
    if (dev == NULL) {
        return;
    }

    dev->sample_lpf_initialized = 0u;
}

void ICM45688_MSPM0_SetAttitudeMode(icm45688_mspm0_t *dev,
                                    icm45688_mspm0_attitude_mode_t mode)
{
    if (dev == NULL) {
        return;
    }
    if (mode != ICM45688_MSPM0_ATTITUDE_COMPLEMENTARY &&
        mode != ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS) {
        return;
    }

    if (dev->attitude_mode != mode &&
        mode == ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS) {
        quat_from_euler(dev);
    }
    dev->attitude_mode = mode;
}

void ICM45688_MSPM0_SetAccelTrustWindow(icm45688_mspm0_t *dev,
                                        float max_accel_error_g)
{
    if (dev == NULL) {
        return;
    }

    dev->accel_trust_window_g =
        (max_accel_error_g > 0.0f) ? max_accel_error_g : 0.0f;
}

void ICM45688_MSPM0_SetAccelRejection(icm45688_mspm0_t *dev,
                                      float rejection_angle_deg)
{
    float half_sine;

    if (dev == NULL) {
        return;
    }
    if (rejection_angle_deg <= 0.0f) {
        dev->accel_rejection_threshold_sq = 0.0f;
        return;
    }

    rejection_angle_deg = clampf_local(rejection_angle_deg, 0.0f, 90.0f);
    half_sine = 0.5f * sinf(rejection_angle_deg * ICM45688_DEG_TO_RAD);
    dev->accel_rejection_threshold_sq = half_sine * half_sine;
}

void ICM45688_MSPM0_SetMahonyGains(icm45688_mspm0_t *dev,
                                   float kp,
                                   float ki)
{
    if (dev == NULL) {
        return;
    }

    if (kp >= 0.0f) {
        dev->mahony_kp = kp;
    }
    if (ki >= 0.0f) {
        dev->mahony_ki = ki;
    }
}

void ICM45688_MSPM0_ResetAttitude(icm45688_mspm0_t *dev)
{
    if (dev == NULL) {
        return;
    }

    dev->pitch_deg = 0.0f;
    dev->roll_deg = 0.0f;
    dev->yaw_deg = 0.0f;
    dev->mahony_integral_x = 0.0f;
    dev->mahony_integral_y = 0.0f;
    dev->mahony_integral_z = 0.0f;
    dev->quat_w = 1.0f;
    dev->quat_x = 0.0f;
    dev->quat_y = 0.0f;
    dev->quat_z = 0.0f;
}

void ICM45688_MSPM0_GetQuaternion(const icm45688_mspm0_t *dev,
                                  float *w,
                                  float *x,
                                  float *y,
                                  float *z)
{
    if (dev == NULL) {
        return;
    }

    if (w != NULL) {
        *w = dev->quat_w;
    }
    if (x != NULL) {
        *x = dev->quat_x;
    }
    if (y != NULL) {
        *y = dev->quat_y;
    }
    if (z != NULL) {
        *z = dev->quat_z;
    }
}

void ICM45688_MSPM0_SetGyroBias(icm45688_mspm0_t *dev,
                                float gx_bias_dps,
                                float gy_bias_dps,
                                float gz_bias_dps)
{
    if (dev == NULL) {
        return;
    }

    dev->gyro_bias_x_dps = gx_bias_dps;
    dev->gyro_bias_y_dps = gy_bias_dps;
    dev->gyro_bias_z_dps = gz_bias_dps;
    dev->gyro_bias_stationary_samples = 0u;
}

void ICM45688_MSPM0_SetGyroScale(icm45688_mspm0_t *dev,
                                 float gx_scale,
                                 float gy_scale,
                                 float gz_scale)
{
    if (dev == NULL) {
        return;
    }

    if (gx_scale > 0.0f) {
        dev->gyro_scale_x = gx_scale;
    }
    if (gy_scale > 0.0f) {
        dev->gyro_scale_y = gy_scale;
    }
    if (gz_scale > 0.0f) {
        dev->gyro_scale_z = gz_scale;
    }
}

void ICM45688_MSPM0_SetStationaryDetection(icm45688_mspm0_t *dev,
                                           float gyro_threshold_dps,
                                           float accel_threshold_g,
                                           uint16_t hold_samples)
{
    if (dev == NULL) {
        return;
    }

    if (gyro_threshold_dps > 0.0f) {
        dev->stationary_gyro_threshold_dps = gyro_threshold_dps;
    }
    if (accel_threshold_g >= 0.0f) {
        dev->stationary_accel_threshold_g = accel_threshold_g;
    }
    dev->stationary_hold_samples = (hold_samples == 0u) ? 1u : hold_samples;
    dev->stationary = 0u;
    dev->stationary_sample_count = 0u;
    dev->gyro_bias_stationary_samples = 0u;
}

void ICM45688_MSPM0_SetGyroBiasAutoUpdate(icm45688_mspm0_t *dev,
                                          uint8_t enable,
                                          float learning_rate)
{
    if (dev == NULL) {
        return;
    }

    dev->gyro_bias_auto_update = enable ? 1u : 0u;
    dev->gyro_bias_stationary_samples = 0u;
    if (learning_rate >= 0.0f && learning_rate <= 1.0f) {
        dev->gyro_bias_learning_rate = learning_rate;
    }
}

int ICM45688_MSPM0_ReadWhoAmI(icm45688_mspm0_t *dev, uint8_t *whoami)
{
    int status;

    if (dev == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }
    status = inv_imu_get_who_am_i(&dev->tdk, whoami);
    if (status == INV_IMU_OK && whoami != NULL) {
        dev->last_who_am_i = *whoami;
    }
    return status;
}

int ICM45688_MSPM0_Begin(icm45688_mspm0_t *dev)
{
    int status = INV_IMU_OK;
    uint8_t who = 0u;
    int1_config0_t int1_config0 = {0};
    int1_config1_t int1_config1 = {0};
    inv_imu_int_pin_config_t int1_pin_config;
    inv_imu_int_state_t int1_status;

    if (dev == NULL || dev->bus.inst == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }

    dev->ready = 0u;
    clear_measurements(dev);
    icm_sleep_us(3000u);

    {
        drive_config0_t drive_config0;

        drive_config0.pads_spi_slew = DRIVE_CONFIG0_PADS_SPI_SLEW_TYP_10NS;
        status |= inv_imu_write_reg(&dev->tdk, DRIVE_CONFIG0, 1,
                                    (uint8_t *)&drive_config0);
        if (status != INV_IMU_OK) {
            return status;
        }
        icm_sleep_us(2u);
    }

    status |= ICM45688_MSPM0_ReadWhoAmI(dev, &who);
    if (status != INV_IMU_OK || who != INV_IMU_WHOAMI) {
        return INV_IMU_ERROR;
    }

    status |= inv_imu_soft_reset(&dev->tdk);
    status |= inv_imu_set_accel_fsr(&dev->tdk, ACCEL_CONFIG0_ACCEL_UI_FS_SEL_16_G);
    status |= inv_imu_set_accel_frequency(&dev->tdk, ACCEL_CONFIG0_ACCEL_ODR_200_HZ);
    status |= inv_imu_set_gyro_fsr(&dev->tdk, GYRO_CONFIG0_GYRO_UI_FS_SEL_1000_DPS);
    status |= inv_imu_set_gyro_frequency(&dev->tdk, GYRO_CONFIG0_GYRO_ODR_200_HZ);
    status |= inv_imu_set_accel_ln_bw(
        &dev->tdk, IPREG_SYS2_REG_131_ACCEL_UI_LPFBW_DIV_4);
    status |= inv_imu_set_gyro_ln_bw(
        &dev->tdk, IPREG_SYS1_REG_172_GYRO_UI_LPFBW_DIV_4);
    status |= inv_imu_set_accel_mode(&dev->tdk, PWR_MGMT0_ACCEL_MODE_LN);
    status |= inv_imu_set_gyro_mode(&dev->tdk, PWR_MGMT0_GYRO_MODE_LN);
    int1_config0.int1_status_en_drdy = 1u;
    status |= inv_imu_write_reg(&dev->tdk, INT1_CONFIG0, 1,
                                (uint8_t *)&int1_config0);
    status |= inv_imu_write_reg(&dev->tdk, INT1_CONFIG1, 1,
                                (uint8_t *)&int1_config1);
    int1_pin_config.int_polarity = INTX_CONFIG2_INTX_POLARITY_HIGH;
    int1_pin_config.int_mode = INTX_CONFIG2_INTX_MODE_LATCH;
    int1_pin_config.int_drive = INTX_CONFIG2_INTX_DRIVE_PP;
    status |= inv_imu_set_pin_config_int(&dev->tdk, INV_IMU_INT1,
                                         &int1_pin_config);
    if (status != INV_IMU_OK) {
        return status;
    }

    icm_sleep_us(GYR_STARTUP_TIME_US);
    memset(&int1_status, 0, sizeof(int1_status));
    status |= inv_imu_get_int_status(&dev->tdk, INV_IMU_INT1, &int1_status);
    if (status != INV_IMU_OK) {
        return status;
    }
    dev->ready = 1u;
    return INV_IMU_OK;
}

int ICM45688_MSPM0_GetDataReady(icm45688_mspm0_t *dev, uint8_t *data_ready)
{
    inv_imu_int_state_t int1_status;
    int status;

    if (dev == NULL || data_ready == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }
    if (!dev->ready) {
        return INV_IMU_ERROR;
    }

    memset(&int1_status, 0, sizeof(int1_status));
    status = inv_imu_get_int_status(&dev->tdk, INV_IMU_INT1, &int1_status);
    if (status == INV_IMU_OK) {
        *data_ready = int1_status.INV_UI_DRDY ? 1u : 0u;
    }

    return status;
}

int ICM45688_MSPM0_Update(icm45688_mspm0_t *dev)
{
    inv_imu_sensor_data_t raw;
    float raw_ax_g;
    float raw_ay_g;
    float raw_az_g;
    float raw_gx_dps;
    float raw_gy_dps;
    float raw_gz_dps;
    float old_bias_x_dps;
    float old_bias_y_dps;
    float old_bias_z_dps;

    if (dev == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }
    if (!dev->ready) {
        return INV_IMU_ERROR;
    }

    int status = inv_imu_get_register_data(&dev->tdk, &raw);
    if (status != INV_IMU_OK) {
        return status;
    }

    raw_ax_g = ((float)raw.accel_data[0] * dev->accel_fsr_g) / ICM45688_RAW_MAX;
    raw_ay_g = ((float)raw.accel_data[1] * dev->accel_fsr_g) / ICM45688_RAW_MAX;
    raw_az_g = ((float)raw.accel_data[2] * dev->accel_fsr_g) / ICM45688_RAW_MAX;
    raw_gx_dps = ((float)raw.gyro_data[0] * dev->gyro_fsr_dps) / ICM45688_RAW_MAX;
    raw_gy_dps = ((float)raw.gyro_data[1] * dev->gyro_fsr_dps) / ICM45688_RAW_MAX;
    raw_gz_dps = ((float)raw.gyro_data[2] * dev->gyro_fsr_dps) / ICM45688_RAW_MAX;
    apply_sample_lowpass(dev,
                         raw_ax_g,
                         raw_ay_g,
                         raw_az_g,
                         (raw_gx_dps - dev->gyro_bias_x_dps) * dev->gyro_scale_x,
                         (raw_gy_dps - dev->gyro_bias_y_dps) * dev->gyro_scale_y,
                         (raw_gz_dps - dev->gyro_bias_z_dps) * dev->gyro_scale_z);
    dev->temp_c = 25.0f + ((float)raw.temp_data / 128.0f);
    update_stationary_state(dev);
    old_bias_x_dps = dev->gyro_bias_x_dps;
    old_bias_y_dps = dev->gyro_bias_y_dps;
    old_bias_z_dps = dev->gyro_bias_z_dps;
    update_auto_gyro_bias(dev, raw_gx_dps, raw_gy_dps, raw_gz_dps);
    dev->gx_dps -= (dev->gyro_bias_x_dps - old_bias_x_dps) * dev->gyro_scale_x;
    dev->gy_dps -= (dev->gyro_bias_y_dps - old_bias_y_dps) * dev->gyro_scale_y;
    dev->gz_dps -= (dev->gyro_bias_z_dps - old_bias_z_dps) * dev->gyro_scale_z;

    if (dev->attitude_mode == ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS) {
        update_mahony_attitude(dev);
    } else {
        update_complementary_attitude(dev);
    }

    return INV_IMU_OK;
}

uint8_t ICM45688_MSPM0_IsReady(const icm45688_mspm0_t *dev)
{
    return (dev != NULL) ? dev->ready : 0u;
}

uint8_t ICM45688_MSPM0_IsStationary(const icm45688_mspm0_t *dev)
{
    return (dev != NULL) ? dev->stationary : 0u;
}
