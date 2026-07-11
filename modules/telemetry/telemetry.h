#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

typedef struct {
    int16_t left_pwm;
    int16_t right_pwm;
    uint32_t hold_ms;
} TelemetryDutyRequest_t;

typedef enum {
    TELEMETRY_IMU_CAL_NONE = 0,
    TELEMETRY_IMU_CAL_START,
    TELEMETRY_IMU_CAL_CANCEL,
} TelemetryImuCalRequest_t;

void Telemetry_Init(void);
void Telemetry_BindChassis(const uint8_t *state,
                           const float *x_cm, const float *y_cm,
                           const float *theta_rad,
                           const float *target_l, const float *target_r,
                           const float *meas_l, const float *meas_r,
                           const float *line_turn);
void Telemetry_BindLine(const uint16_t *raw, uint8_t raw_count,
                        const uint16_t *contrast, const uint16_t *strength,
                        const float *bias, const uint8_t *on_line);
void Telemetry_BindImu(const float *pitch_deg, const float *roll_deg,
                       const float *yaw_deg,
                       const float *gx_dps, const float *gy_dps,
                       const float *gz_dps,
                       const float *ax_g, const float *ay_g,
                       const float *az_g, const uint8_t *ready);
void Telemetry_Tick(uint32_t ts_ms);
void Telemetry_OnRxByte(uint8_t byte);

uint8_t Telemetry_ConsumeStartRequest(void);
uint8_t Telemetry_ConsumeStopRequest(void);
uint8_t Telemetry_ConsumeDutyRequest(TelemetryDutyRequest_t *request);
TelemetryImuCalRequest_t Telemetry_ConsumeImuCalRequest(void);

#endif /* TELEMETRY_H */
