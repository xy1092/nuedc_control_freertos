#include "telemetry.h"
#include "bsp_uart.h"
#include <stdlib.h>
#include <string.h>

#define CMD_BUF_LEN 80u
#define DEG_PER_RAD 57.2957795f

typedef struct {
    const uint8_t *state;
    const float *x_cm;
    const float *y_cm;
    const float *theta_rad;
    const float *target_l;
    const float *target_r;
    const float *meas_l;
    const float *meas_r;
    const float *line_turn;
    uint8_t valid;
} TelemetryChassisBind_t;

typedef struct {
    const uint16_t *raw;
    uint8_t raw_count;
    const uint16_t *contrast;
    const uint16_t *strength;
    const float *bias;
    const uint8_t *on_line;
    uint8_t valid;
} TelemetryLineBind_t;

typedef struct {
    const float *pitch_deg;
    const float *roll_deg;
    const float *yaw_deg;
    const float *gx_dps;
    const float *gy_dps;
    const float *gz_dps;
    const float *ax_g;
    const float *ay_g;
    const float *az_g;
    const uint8_t *ready;
    uint8_t valid;
} TelemetryImuBind_t;

static TelemetryChassisBind_t s_chassis;
static TelemetryLineBind_t s_line;
static TelemetryImuBind_t s_imu;

static uint16_t s_period_ms = 10u;
static uint32_t s_last_emit_ms = 0u;
static uint8_t s_emit_primed = 0u;
static uint8_t s_enabled = 1u;
static uint8_t s_raw_line_enabled = 0u;
static uint8_t s_raw_imu_enabled = 0u;

static volatile uint8_t s_start_request = 0u;
static volatile uint8_t s_stop_request = 0u;
static volatile uint8_t s_duty_request_pending = 0u;
static volatile TelemetryImuCalRequest_t s_imu_cal_request =
    TELEMETRY_IMU_CAL_NONE;
static TelemetryDutyRequest_t s_duty_request;

static char s_cmd_buf[CMD_BUF_LEN];
static uint8_t s_cmd_idx = 0u;

void Telemetry_BindChassis(const uint8_t *state,
                           const float *x_cm, const float *y_cm,
                           const float *theta_rad,
                           const float *target_l, const float *target_r,
                           const float *meas_l, const float *meas_r,
                           const float *line_turn)
{
    s_chassis.state = state;
    s_chassis.x_cm = x_cm;
    s_chassis.y_cm = y_cm;
    s_chassis.theta_rad = theta_rad;
    s_chassis.target_l = target_l;
    s_chassis.target_r = target_r;
    s_chassis.meas_l = meas_l;
    s_chassis.meas_r = meas_r;
    s_chassis.line_turn = line_turn;
    s_chassis.valid = (state && x_cm && y_cm && theta_rad &&
                       target_l && target_r && meas_l && meas_r &&
                       line_turn) ? 1u : 0u;
}

void Telemetry_BindLine(const uint16_t *raw, uint8_t raw_count,
                        const uint16_t *contrast, const uint16_t *strength,
                        const float *bias, const uint8_t *on_line)
{
    s_line.raw = raw;
    s_line.raw_count = raw_count;
    s_line.contrast = contrast;
    s_line.strength = strength;
    s_line.bias = bias;
    s_line.on_line = on_line;
    s_line.valid = (raw && raw_count > 0u && contrast && strength &&
                    bias && on_line) ? 1u : 0u;
}

void Telemetry_BindImu(const float *pitch_deg, const float *roll_deg,
                       const float *yaw_deg,
                       const float *gx_dps, const float *gy_dps,
                       const float *gz_dps,
                       const float *ax_g, const float *ay_g,
                       const float *az_g, const uint8_t *ready)
{
    s_imu.pitch_deg = pitch_deg;
    s_imu.roll_deg = roll_deg;
    s_imu.yaw_deg = yaw_deg;
    s_imu.gx_dps = gx_dps;
    s_imu.gy_dps = gy_dps;
    s_imu.gz_dps = gz_dps;
    s_imu.ax_g = ax_g;
    s_imu.ay_g = ay_g;
    s_imu.az_g = az_g;
    s_imu.ready = ready;
    s_imu.valid = (pitch_deg && roll_deg && yaw_deg &&
                   gx_dps && gy_dps && gz_dps &&
                   ax_g && ay_g && az_g && ready) ? 1u : 0u;
}

void Telemetry_Init(void)
{
    BSP_Uart_SetRxCallback(0, Telemetry_OnRxByte);
}

static uint8_t consume_flag(volatile uint8_t *flag)
{
    uint8_t value = *flag;
    *flag = 0u;
    return value;
}

uint8_t Telemetry_ConsumeStartRequest(void)
{
    return consume_flag(&s_start_request);
}

uint8_t Telemetry_ConsumeStopRequest(void)
{
    return consume_flag(&s_stop_request);
}

uint8_t Telemetry_ConsumeDutyRequest(TelemetryDutyRequest_t *request)
{
    if (!s_duty_request_pending) {
        return 0u;
    }
    if (request) {
        *request = s_duty_request;
    }
    s_duty_request_pending = 0u;
    return 1u;
}

TelemetryImuCalRequest_t Telemetry_ConsumeImuCalRequest(void)
{
    TelemetryImuCalRequest_t request = s_imu_cal_request;
    s_imu_cal_request = TELEMETRY_IMU_CAL_NONE;
    return request;
}

static void emit_chassis(uint32_t ts_ms)
{
    if (!s_chassis.valid) {
        return;
    }

    BSP_Uart_Printf("$S,%lu,%u,%.2f,%.2f,%.2f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                    (unsigned long)ts_ms,
                    (unsigned)*s_chassis.state,
                    *s_chassis.x_cm,
                    *s_chassis.y_cm,
                    *s_chassis.theta_rad * DEG_PER_RAD,
                    *s_chassis.target_l,
                    *s_chassis.target_r,
                    *s_chassis.meas_l,
                    *s_chassis.meas_r,
                    *s_chassis.line_turn);
}

static void emit_line(uint32_t ts_ms)
{
    if (!s_line.valid || s_line.raw_count < 7u) {
        return;
    }

    if (s_line.raw_count >= 8u) {
        BSP_Uart_Printf("$L,%lu,%.3f,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
                        (unsigned long)ts_ms,
                        *s_line.bias,
                        (unsigned)*s_line.contrast,
                        (unsigned)*s_line.strength,
                        (unsigned)*s_line.on_line,
                        (unsigned)s_line.raw[0],
                        (unsigned)s_line.raw[1],
                        (unsigned)s_line.raw[2],
                        (unsigned)s_line.raw[3],
                        (unsigned)s_line.raw[4],
                        (unsigned)s_line.raw[5],
                        (unsigned)s_line.raw[6],
                        (unsigned)s_line.raw[7]);
    } else {
        BSP_Uart_Printf("$L,%lu,%.3f,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
                        (unsigned long)ts_ms,
                        *s_line.bias,
                        (unsigned)*s_line.contrast,
                        (unsigned)*s_line.strength,
                        (unsigned)*s_line.on_line,
                        (unsigned)s_line.raw[0],
                        (unsigned)s_line.raw[1],
                        (unsigned)s_line.raw[2],
                        (unsigned)s_line.raw[3],
                        (unsigned)s_line.raw[4],
                        (unsigned)s_line.raw[5],
                        (unsigned)s_line.raw[6]);
    }
}

static void emit_imu(uint32_t ts_ms)
{
    if (!s_imu.valid) {
        return;
    }

    BSP_Uart_Printf("$I,%lu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.4f,%.4f,%u\r\n",
                    (unsigned long)ts_ms,
                    *s_imu.pitch_deg,
                    *s_imu.roll_deg,
                    *s_imu.yaw_deg,
                    *s_imu.gx_dps,
                    *s_imu.gy_dps,
                    *s_imu.gz_dps,
                    *s_imu.ax_g,
                    *s_imu.ay_g,
                    *s_imu.az_g,
                    (unsigned)*s_imu.ready);
}

void Telemetry_Tick(uint32_t ts_ms)
{
    if (!s_enabled && !s_raw_imu_enabled) {
        return;
    }
    if (!s_emit_primed) {
        s_emit_primed = 1u;
        s_last_emit_ms = ts_ms;
    } else if ((uint32_t)(ts_ms - s_last_emit_ms) < s_period_ms) {
        return;
    } else {
        s_last_emit_ms = ts_ms;
    }

    if (s_enabled) {
        emit_chassis(ts_ms);
        if (s_raw_line_enabled) {
            emit_line(ts_ms);
        }
    }
    if (s_raw_imu_enabled) {
        emit_imu(ts_ms);
    }
}

static int parse_i16(const char *s)
{
    return (int)strtol(s, 0, 10);
}

static void handle_duty(char *args)
{
    char *p1;
    char *p2;

    if (!args) {
        BSP_Uart_Printf("$ERR,duty,parse\r\n");
        return;
    }
    p1 = strchr(args, ',');
    if (!p1) {
        BSP_Uart_Printf("$ERR,duty,parse\r\n");
        return;
    }
    *p1++ = 0;
    p2 = strchr(p1, ',');
    if (p2) {
        *p2++ = 0;
    }

    s_duty_request.left_pwm = (int16_t)parse_i16(args);
    s_duty_request.right_pwm = (int16_t)parse_i16(p1);
    s_duty_request.hold_ms = p2 ? (uint32_t)strtoul(p2, 0, 10) : 800u;
    s_duty_request_pending = 1u;
    BSP_Uart_Printf("$OK,duty,%d,%d,%lu\r\n",
                    (int)s_duty_request.left_pwm,
                    (int)s_duty_request.right_pwm,
                    (unsigned long)s_duty_request.hold_ms);
}

static void handle_line(char *line)
{
    char *p = line;
    while (*p && *p != '\r' && *p != '\n') {
        ++p;
    }
    *p = 0;

    if (!strcmp(line, "$START") || !strcmp(line, "$RUN")) {
        s_start_request = 1u;
        BSP_Uart_Printf("$OK,start\r\n");
    } else if (!strcmp(line, "$STOP")) {
        s_stop_request = 1u;
        BSP_Uart_Printf("$OK,stop\r\n");
    } else if (!strcmp(line, "$PAUSE")) {
        s_enabled = 0u;
        BSP_Uart_Printf("$OK,pause\r\n");
    } else if (!strcmp(line, "$RESUME")) {
        s_enabled = 1u;
        BSP_Uart_Printf("$OK,resume\r\n");
    } else if (!strncmp(line, "$RATE,", 6)) {
        int hz = atoi(line + 6);
        if (hz >= 1 && hz <= 500) {
            s_period_ms = (uint16_t)(1000 / hz);
            if (s_period_ms == 0u) {
                s_period_ms = 1u;
            }
            BSP_Uart_Printf("$OK,rate,%d\r\n", hz);
        } else {
            BSP_Uart_Printf("$ERR,rate,range\r\n");
        }
    } else if (!strncmp(line, "$RAWLINE,", 9)) {
        s_raw_line_enabled = atoi(line + 9) ? 1u : 0u;
        BSP_Uart_Printf("$OK,rawline,%u\r\n", (unsigned)s_raw_line_enabled);
    } else if (!strncmp(line, "$RAWIMU,", 8)) {
        s_raw_imu_enabled = atoi(line + 8) ? 1u : 0u;
        BSP_Uart_Printf("$OK,rawimu,%u\r\n", (unsigned)s_raw_imu_enabled);
    } else if (!strncmp(line, "$DUTY,", 6)) {
        handle_duty(line + 6);
    } else if (!strcmp(line, "$IMUCAL,START")) {
        s_imu_cal_request = TELEMETRY_IMU_CAL_START;
        BSP_Uart_Printf("$OK,imucal,start\r\n");
    } else if (!strcmp(line, "$IMUCAL,CANCEL")) {
        s_imu_cal_request = TELEMETRY_IMU_CAL_CANCEL;
        BSP_Uart_Printf("$OK,imucal,cancel\r\n");
    } else if (!strcmp(line, "$DUMP")) {
        BSP_Uart_Printf("$CFG,rate_ms,%u\r\n", (unsigned)s_period_ms);
        BSP_Uart_Printf("$CFG,enabled,%u\r\n", (unsigned)s_enabled);
        BSP_Uart_Printf("$CFG,rawline,%u\r\n", (unsigned)s_raw_line_enabled);
        BSP_Uart_Printf("$CFG,rawimu,%u\r\n", (unsigned)s_raw_imu_enabled);
    } else if (line[0] != 0) {
        BSP_Uart_Printf("$ERR,unknown\r\n");
    }
}

void Telemetry_OnRxByte(uint8_t byte)
{
    if (byte == 's') {
        s_start_request = 1u;
        return;
    }
    if (byte == 'x') {
        s_stop_request = 1u;
        return;
    }
    if (byte == '\n') {
        s_cmd_buf[s_cmd_idx] = 0;
        handle_line(s_cmd_buf);
        s_cmd_idx = 0u;
        return;
    }
    if (byte == '\r') {
        return;
    }
    if (s_cmd_idx < (CMD_BUF_LEN - 1u)) {
        s_cmd_buf[s_cmd_idx++] = (char)byte;
    } else {
        s_cmd_idx = 0u;
    }
}
