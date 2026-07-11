#include "imu.h"
#include "imu_profile.h"
#include "bsp_imu.h"
#include "dev_icm45688.h"
#include "fault_manager.h"
#include "health_monitor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <string.h>

#define ICM45688_DATA_TIMEOUT_MS        50u
#define ICM45688_EXPECTED_WHOAMI        0xE9u
#define ICM45688_INIT_RETRIES           5u
#define GYRO_CAL_RETRY_DELAY_MS         500u
#define YAW_CAL_FOUR_LAP_DEG            1440.0f
#define YAW_CAL_MOTION_START_DPS         5.0f
#define YAW_CAL_CAPTURE_MIN_RATIO        0.90f
#define YAW_CAL_SETTLE_SAMPLES           40u
#define YAW_CAL_SCALE_MIN                0.85f
#define YAW_CAL_SCALE_MAX                1.15f
#define YAW_CAL_DIRECTION_MISMATCH_MAX   0.02f

typedef struct {
    ImuYawCalState_t state;
    uint16_t settle_samples;
    float phase_origin_deg;
    float positive_measured_deg;
    float negative_measured_deg;
    float positive_scale;
    float negative_scale;
    float result_scale;
    float previous_scale;
} YawCalibration_t;

static icm45688_mspm0_t s_imu;
static ImuSnapshot_t s_snapshot;
static YawCalibration_t s_yaw_cal;
static volatile uint8_t s_yaw_cal_start_requested;
static volatile uint8_t s_yaw_cal_cancel_requested;

static uint32_t now_ms(void)
{
    return (uint32_t)xTaskGetTickCount();
}

static void heartbeat(void)
{
    System_Health_Beat(HEALTH_TASK_IMU, now_ms());
}

static void delay_with_heartbeat(uint32_t delay_ms)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t delay_ticks = pdMS_TO_TICKS(delay_ms);

    while ((xTaskGetTickCount() - start) < delay_ticks) {
        heartbeat();
        vTaskDelay(pdMS_TO_TICKS(10u));
    }
}

static float abs_float(float value)
{
    return (value < 0.0f) ? -value : value;
}

static float sample_stddev(float sum, float sum_sq, uint32_t count)
{
    float mean;
    float variance;

    if (count == 0u) {
        return 0.0f;
    }

    mean = sum / (float)count;
    variance = (sum_sq / (float)count) - (mean * mean);
    if (variance < 0.0f) {
        variance = 0.0f;
    }
    return sqrtf(variance);
}

static void publish_snapshot(ImuState_t state, int status)
{
    taskENTER_CRITICAL();
    s_snapshot.sequence++;
    s_snapshot.timestamp_ms = now_ms();
    s_snapshot.accel_g[0] = s_imu.ax_g;
    s_snapshot.accel_g[1] = s_imu.ay_g;
    s_snapshot.accel_g[2] = s_imu.az_g;
    s_snapshot.gyro_dps[0] = s_imu.gx_dps;
    s_snapshot.gyro_dps[1] = s_imu.gy_dps;
    s_snapshot.gyro_dps[2] = s_imu.gz_dps;
    s_snapshot.temperature_c = s_imu.temp_c;
    s_snapshot.pitch_deg = s_imu.pitch_deg;
    s_snapshot.roll_deg = s_imu.roll_deg;
    s_snapshot.yaw_deg = s_imu.yaw_deg;
    s_snapshot.gyro_bias_z_dps = s_imu.gyro_bias_z_dps;
    s_snapshot.gyro_scale_z = s_imu.gyro_scale_z;
    s_snapshot.stationary = ICM45688_MSPM0_IsStationary(&s_imu);
    s_snapshot.ready = ICM45688_MSPM0_IsReady(&s_imu);
    s_snapshot.status = status;
    s_snapshot.state = state;
    s_snapshot.yaw_cal_state = s_yaw_cal.state;
    taskEXIT_CRITICAL();
}

static int wait_and_update_imu(uint32_t timeout_ms)
{
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    for (;;) {
        if (BSP_IMU_DataReady()) {
            uint8_t data_ready = 0u;
            int status = ICM45688_MSPM0_GetDataReady(&s_imu, &data_ready);

            if (status != INV_IMU_OK) {
                return status;
            }
            if (data_ready) {
                return ICM45688_MSPM0_Update(&s_imu);
            }
        }
        if ((xTaskGetTickCount() - start_tick) >= timeout_ticks) {
            return INV_IMU_ERROR_TIMEOUT;
        }
        heartbeat();
        vTaskDelay(pdMS_TO_TICKS(1u));
    }
}

static void configure_imu_handle(void)
{
    const ImuCalibratedProfile_t *profile = &g_imu_calibrated_profile;

    ICM45688_MSPM0_InitHandleSPI(&s_imu, BSP_IMU_GetBus());
    ICM45688_MSPM0_SetFilter(&s_imu, profile->sample_period_s,
                             profile->filter_alpha);
    ICM45688_MSPM0_SetSampleLowPass(&s_imu, profile->accel_lpf_hz,
                                    profile->gyro_lpf_hz);
    ICM45688_MSPM0_SetStationaryDetection(
        &s_imu, profile->stationary_gyro_dps, profile->stationary_accel_g,
        profile->stationary_hold_samples);
    ICM45688_MSPM0_SetAttitudeMode(
        &s_imu, ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS);
    ICM45688_MSPM0_SetAccelTrustWindow(&s_imu,
                                       profile->accel_trust_window_g);
    ICM45688_MSPM0_SetAccelRejection(&s_imu,
                                     profile->accel_rejection_deg);
    ICM45688_MSPM0_SetMahonyGains(&s_imu, profile->mahony_kp,
                                  profile->mahony_ki);
}

static int calibrate_gyro_bias(void)
{
    const ImuCalibratedProfile_t *profile = &g_imu_calibrated_profile;

    for (;;) {
        float gx_sum = 0.0f;
        float gy_sum = 0.0f;
        float gz_sum = 0.0f;
        float gx_sq_sum = 0.0f;
        float gy_sq_sum = 0.0f;
        float gz_sq_sum = 0.0f;
        float gx_stddev;
        float gy_stddev;
        float gz_stddev;

        ICM45688_MSPM0_SetGyroScale(&s_imu, 1.0f, 1.0f, 1.0f);
        ICM45688_MSPM0_SetGyroBias(&s_imu, 0.0f, 0.0f, 0.0f);
        ICM45688_MSPM0_SetGyroBiasAutoUpdate(&s_imu, 0u, 0.0f);
        ICM45688_MSPM0_ResetSampleLowPass(&s_imu);
        ICM45688_MSPM0_ResetAttitude(&s_imu);

        for (uint32_t i = 0u;
             i < (uint32_t)(profile->gyro_cal_warmup_samples +
                            profile->gyro_cal_samples);
             ++i) {
            int status = wait_and_update_imu(ICM45688_DATA_TIMEOUT_MS);

            if (status != INV_IMU_OK) {
                return status;
            }
            heartbeat();
            if (i < profile->gyro_cal_warmup_samples) {
                continue;
            }
            gx_sum += s_imu.gx_dps;
            gy_sum += s_imu.gy_dps;
            gz_sum += s_imu.gz_dps;
            gx_sq_sum += s_imu.gx_dps * s_imu.gx_dps;
            gy_sq_sum += s_imu.gy_dps * s_imu.gy_dps;
            gz_sq_sum += s_imu.gz_dps * s_imu.gz_dps;
        }

        gx_stddev = sample_stddev(gx_sum, gx_sq_sum,
                                  profile->gyro_cal_samples);
        gy_stddev = sample_stddev(gy_sum, gy_sq_sum,
                                  profile->gyro_cal_samples);
        gz_stddev = sample_stddev(gz_sum, gz_sq_sum,
                                  profile->gyro_cal_samples);
        if (gx_stddev > profile->gyro_cal_stddev_limit_dps ||
            gy_stddev > profile->gyro_cal_stddev_limit_dps ||
            gz_stddev > profile->gyro_cal_stddev_limit_dps) {
            delay_with_heartbeat(GYRO_CAL_RETRY_DELAY_MS);
            continue;
        }

        ICM45688_MSPM0_SetGyroBias(
            &s_imu,
            gx_sum / (float)profile->gyro_cal_samples,
            gy_sum / (float)profile->gyro_cal_samples,
            gz_sum / (float)profile->gyro_cal_samples);
        ICM45688_MSPM0_SetGyroScale(&s_imu, 1.0f, 1.0f,
                                    profile->gyro_z_scale);
        ICM45688_MSPM0_SetGyroBiasAutoUpdate(
            &s_imu, 1u, profile->gyro_bias_learning_rate);
        ICM45688_MSPM0_ResetSampleLowPass(&s_imu);
        ICM45688_MSPM0_ResetAttitude(&s_imu);
        return INV_IMU_OK;
    }
}

static uint8_t scale_is_valid(float scale)
{
    return (scale >= YAW_CAL_SCALE_MIN && scale <= YAW_CAL_SCALE_MAX) ? 1u : 0u;
}

static void fail_yaw_calibration(void)
{
    s_yaw_cal.state = IMU_YAW_CAL_FAILED;
    s_yaw_cal.result_scale = s_yaw_cal.previous_scale;
    ICM45688_MSPM0_SetGyroScale(&s_imu, s_imu.gyro_scale_x,
                                s_imu.gyro_scale_y,
                                s_yaw_cal.previous_scale);
    ICM45688_MSPM0_ResetAttitude(&s_imu);
}

static void start_yaw_calibration(void)
{
    float previous_scale = s_imu.gyro_scale_z;

    memset(&s_yaw_cal, 0, sizeof(s_yaw_cal));
    s_yaw_cal.state = IMU_YAW_CAL_WAIT_STILL;
    s_yaw_cal.result_scale = 1.0f;
    s_yaw_cal.previous_scale = previous_scale;
    ICM45688_MSPM0_SetGyroScale(&s_imu, s_imu.gyro_scale_x,
                                s_imu.gyro_scale_y, 1.0f);
    ICM45688_MSPM0_ResetSampleLowPass(&s_imu);
    ICM45688_MSPM0_ResetAttitude(&s_imu);
}

static void complete_yaw_calibration(void)
{
    float scale_sum = s_yaw_cal.positive_scale + s_yaw_cal.negative_scale;
    float scale_difference = abs_float(s_yaw_cal.positive_scale -
                                       s_yaw_cal.negative_scale);

    if (!scale_is_valid(s_yaw_cal.positive_scale) ||
        !scale_is_valid(s_yaw_cal.negative_scale) ||
        scale_sum <= 0.0f ||
        (2.0f * scale_difference / scale_sum) >
            YAW_CAL_DIRECTION_MISMATCH_MAX) {
        fail_yaw_calibration();
        return;
    }

    s_yaw_cal.result_scale =
        (2.0f * YAW_CAL_FOUR_LAP_DEG) /
        (s_yaw_cal.positive_measured_deg - s_yaw_cal.negative_measured_deg);
    if (!scale_is_valid(s_yaw_cal.result_scale)) {
        fail_yaw_calibration();
        return;
    }
    ICM45688_MSPM0_SetGyroScale(&s_imu, s_imu.gyro_scale_x,
                                s_imu.gyro_scale_y,
                                s_yaw_cal.result_scale);
    ICM45688_MSPM0_ResetAttitude(&s_imu);
    s_yaw_cal.phase_origin_deg = 0.0f;
    s_yaw_cal.state = IMU_YAW_CAL_COMPLETE;
}

static void update_yaw_calibration(void)
{
    float phase_yaw_deg = s_imu.yaw_deg - s_yaw_cal.phase_origin_deg;

    switch (s_yaw_cal.state) {
    case IMU_YAW_CAL_WAIT_STILL:
        if (ICM45688_MSPM0_IsStationary(&s_imu)) {
            if (++s_yaw_cal.settle_samples >= YAW_CAL_SETTLE_SAMPLES) {
                ICM45688_MSPM0_ResetAttitude(&s_imu);
                s_yaw_cal.phase_origin_deg = 0.0f;
                s_yaw_cal.settle_samples = 0u;
                s_yaw_cal.state = IMU_YAW_CAL_WAIT_POSITIVE;
            }
        } else {
            s_yaw_cal.settle_samples = 0u;
        }
        break;
    case IMU_YAW_CAL_WAIT_POSITIVE:
        if (s_imu.gz_dps >= YAW_CAL_MOTION_START_DPS) {
            s_yaw_cal.state = IMU_YAW_CAL_POSITIVE_ACTIVE;
        }
        break;
    case IMU_YAW_CAL_POSITIVE_ACTIVE:
        if (ICM45688_MSPM0_IsStationary(&s_imu) &&
            phase_yaw_deg >=
                (YAW_CAL_FOUR_LAP_DEG * YAW_CAL_CAPTURE_MIN_RATIO)) {
            if (++s_yaw_cal.settle_samples >= YAW_CAL_SETTLE_SAMPLES) {
                s_yaw_cal.positive_measured_deg = phase_yaw_deg;
                s_yaw_cal.positive_scale = YAW_CAL_FOUR_LAP_DEG / phase_yaw_deg;
                if (!scale_is_valid(s_yaw_cal.positive_scale)) {
                    fail_yaw_calibration();
                    break;
                }
                s_yaw_cal.phase_origin_deg = s_imu.yaw_deg;
                s_yaw_cal.settle_samples = 0u;
                s_yaw_cal.state = IMU_YAW_CAL_WAIT_NEGATIVE;
            }
        } else {
            s_yaw_cal.settle_samples = 0u;
        }
        break;
    case IMU_YAW_CAL_WAIT_NEGATIVE:
        if (s_imu.gz_dps <= -YAW_CAL_MOTION_START_DPS) {
            s_yaw_cal.state = IMU_YAW_CAL_NEGATIVE_ACTIVE;
        }
        break;
    case IMU_YAW_CAL_NEGATIVE_ACTIVE:
        if (ICM45688_MSPM0_IsStationary(&s_imu) &&
            phase_yaw_deg <=
                -(YAW_CAL_FOUR_LAP_DEG * YAW_CAL_CAPTURE_MIN_RATIO)) {
            if (++s_yaw_cal.settle_samples >= YAW_CAL_SETTLE_SAMPLES) {
                s_yaw_cal.negative_measured_deg = phase_yaw_deg;
                s_yaw_cal.negative_scale =
                    -YAW_CAL_FOUR_LAP_DEG / phase_yaw_deg;
                complete_yaw_calibration();
            }
        } else {
            s_yaw_cal.settle_samples = 0u;
        }
        break;
    default:
        break;
    }
}

static void service_yaw_requests(void)
{
    if (s_yaw_cal_cancel_requested) {
        ImuYawCalState_t previous_state = s_yaw_cal.state;
        float previous_scale = s_yaw_cal.previous_scale;

        s_yaw_cal_cancel_requested = 0u;
        memset(&s_yaw_cal, 0, sizeof(s_yaw_cal));
        if (previous_state != IMU_YAW_CAL_IDLE &&
            previous_state != IMU_YAW_CAL_COMPLETE &&
            previous_scale > 0.0f) {
            ICM45688_MSPM0_SetGyroScale(&s_imu, s_imu.gyro_scale_x,
                                        s_imu.gyro_scale_y, previous_scale);
        }
        ICM45688_MSPM0_ResetAttitude(&s_imu);
    }
    if (s_yaw_cal_start_requested) {
        s_yaw_cal_start_requested = 0u;
        start_yaw_calibration();
    }
}

void IMU_Service_Init(void)
{
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    memset(&s_yaw_cal, 0, sizeof(s_yaw_cal));
}

void IMU_GetSnapshot(ImuSnapshot_t *snapshot)
{
    if (snapshot == 0) {
        return;
    }
    taskENTER_CRITICAL();
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL();
}

void IMU_RequestYawCalibration(void)
{
    s_yaw_cal_start_requested = 1u;
}

void IMU_CancelYawCalibration(void)
{
    s_yaw_cal_cancel_requested = 1u;
}

void IMU_Service_Task(void *argument)
{
    uint8_t whoami = 0u;
    int status = INV_IMU_ERROR;

    (void)argument;
    publish_snapshot(IMU_STATE_INITIALIZING, status);
    for (uint8_t attempt = 1u; attempt <= ICM45688_INIT_RETRIES; ++attempt) {
        configure_imu_handle();
        status = ICM45688_MSPM0_ReadWhoAmI(&s_imu, &whoami);
        if (status == INV_IMU_OK && whoami == ICM45688_EXPECTED_WHOAMI) {
            status = ICM45688_MSPM0_Begin(&s_imu);
        }
        if (status == INV_IMU_OK) {
            break;
        }
        delay_with_heartbeat(200u);
    }
    if (status != INV_IMU_OK) {
        publish_snapshot(IMU_STATE_FAULT, status);
        System_Fault_Raise(SYSTEM_FAULT_IMU, (uint32_t)status);
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
    }

    publish_snapshot(IMU_STATE_CALIBRATING, status);
    status = calibrate_gyro_bias();
    if (status != INV_IMU_OK) {
        publish_snapshot(IMU_STATE_FAULT, status);
        System_Fault_Raise(SYSTEM_FAULT_IMU, (uint32_t)status);
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000u));
        }
    }

    memset(&s_yaw_cal, 0, sizeof(s_yaw_cal));
    for (;;) {
        service_yaw_requests();
        status = wait_and_update_imu(ICM45688_DATA_TIMEOUT_MS);
        if (status == INV_IMU_OK) {
            update_yaw_calibration();
        }
        publish_snapshot(IMU_STATE_READY, status);
        heartbeat();
    }
}
