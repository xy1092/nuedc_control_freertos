#ifndef IMU_H
#define IMU_H

#include <stdint.h>

typedef enum {
    IMU_STATE_OFF = 0,
    IMU_STATE_INITIALIZING,
    IMU_STATE_CALIBRATING,
    IMU_STATE_READY,
    IMU_STATE_FAULT,
} ImuState_t;

typedef enum {
    IMU_YAW_CAL_IDLE = 0,
    IMU_YAW_CAL_WAIT_STILL,
    IMU_YAW_CAL_WAIT_POSITIVE,
    IMU_YAW_CAL_POSITIVE_ACTIVE,
    IMU_YAW_CAL_WAIT_NEGATIVE,
    IMU_YAW_CAL_NEGATIVE_ACTIVE,
    IMU_YAW_CAL_COMPLETE,
    IMU_YAW_CAL_FAILED,
} ImuYawCalState_t;

typedef struct {
    uint32_t sequence;
    uint32_t timestamp_ms;
    float accel_g[3];
    float gyro_dps[3];
    float temperature_c;
    float pitch_deg;
    float roll_deg;
    float yaw_deg;
    float gyro_bias_z_dps;
    float gyro_scale_z;
    int status;
    ImuState_t state;
    ImuYawCalState_t yaw_cal_state;
    uint8_t stationary;
    uint8_t ready;
} ImuSnapshot_t;

void IMU_Service_Init(void);
void IMU_Service_Task(void *argument);
void IMU_GetSnapshot(ImuSnapshot_t *snapshot);
void IMU_RequestYawCalibration(void);
void IMU_CancelYawCalibration(void);

#endif
