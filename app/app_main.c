#include "app_main.h"
#include "app_config.h"
#include "../bsp/bsp_adc.h"
#include "../bsp/bsp_imu.h"
#include "../bsp/bsp_motor.h"
#include "../bsp/bsp_uart.h"
#include "../middleware/control/chassis.h"
#include "../middleware/control/line_tracker.h"
#include "../middleware/pid.h"
#include "../middleware/telemetry.h"
#include "FreeRTOS.h"
#include "task.h"

extern volatile uint32_t g_tick_ms;

static AppState_t s_state = APP_STATE_IDLE;
static uint8_t s_state_id = APP_STATE_IDLE;

static ControlChassis_t s_chassis;
static PID_t s_line_pid;

static float s_line_turn = 0.0f;
static float s_pitch_deg = 0.0f;
static float s_roll_deg = 0.0f;
static float s_yaw_deg = 0.0f;
static float s_gx_dps = 0.0f;
static float s_gy_dps = 0.0f;
static float s_gz_dps = 0.0f;
static float s_ax_g = 0.0f;
static float s_ay_g = 0.0f;
static float s_az_g = 0.0f;
static uint8_t s_imu_ready = 0u;

static const ControlChassisConfig_t k_chassis_config = {
    APP_PULSES_PER_CM,
    APP_WHEEL_BASE_CM,
    APP_OPEN_LOOP_CM_PER_PWM_MS,
    APP_SPEED_OUT_LIMIT,
    APP_FORWARD_MIN_PULSE,
};

static float clampf_local(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static void set_state(AppState_t state)
{
    s_state = state;
    s_state_id = (uint8_t)state;
    if (state != APP_STATE_RUN) {
        Control_Chassis_SetTargetSpeeds(&s_chassis, 0.0f, 0.0f);
        BSP_Motor_SetDuty(0, 0);
        BSP_Motor_SetDuty(1, 0);
    }
}

AppState_t App_GetState(void)
{
    return s_state;
}

static void fast_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        g_tick_ms = (uint32_t)xTaskGetTickCount();
        Control_Chassis_PollFastInputs(&s_chassis);
        BSP_IMU_Update(&s_pitch_deg, &s_roll_deg, &s_yaw_deg);
        BSP_IMU_GetGyro(&s_gx_dps, &s_gy_dps, &s_gz_dps);
        BSP_IMU_GetAccel(&s_ax_g, &s_ay_g, &s_az_g);
        s_imu_ready = BSP_IMU_IsReady();
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_FAST_TASK_PERIOD_MS));
    }
}

static void control_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t now_ms = (uint32_t)xTaskGetTickCount();
        TelemetryDutyRequest_t duty;

        g_tick_ms = now_ms;
        if (Telemetry_ConsumeStartRequest()) {
            ControlPose2D_t zero_pose = { 0.0f, 0.0f, 0.0f };
            Control_Chassis_ResetOdometry(&s_chassis, zero_pose);
            set_state(APP_STATE_RUN);
        }
        if (Telemetry_ConsumeStopRequest()) {
            set_state(APP_STATE_IDLE);
        }
        if (Telemetry_ConsumeDutyRequest(&duty)) {
            Control_Chassis_SetDebugDuty(&s_chassis, duty.left_pwm,
                                         duty.right_pwm, now_ms, duty.hold_ms);
        }

        if (s_state == APP_STATE_RUN) {
            float base = APP_BASE_SPEED_PULSE;
            float turn = Control_LineTracker_Update();

            s_line_turn = clampf_local(turn, -APP_LINE_TURN_LIMIT,
                                       APP_LINE_TURN_LIMIT);
            Control_Chassis_SetTargetSpeeds(&s_chassis,
                                            base - s_line_turn,
                                            base + s_line_turn);
        } else {
            s_line_turn = 0.0f;
            Control_Chassis_SetTargetSpeeds(&s_chassis, 0.0f, 0.0f);
        }

        Control_Chassis_Tick(&s_chassis, (s_state == APP_STATE_RUN) ? 1u : 0u,
                             now_ms);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_CONTROL_TASK_PERIOD_MS));
    }
}

static void telemetry_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t now_ms = (uint32_t)xTaskGetTickCount();
        g_tick_ms = now_ms;
        Telemetry_Tick(now_ms);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_TELEM_TASK_PERIOD_MS));
    }
}

void App_Init(void)
{
    const ControlLineTrackInfo_t *line_info;

    BSP_Motor_Init();
    BSP_Uart_Init();
    BSP_Adc_Init();
    BSP_IMU_Init();

    Control_Chassis_Init(&s_chassis, &k_chassis_config,
                         APP_SPEED_PID_KP, APP_SPEED_PID_KI,
                         APP_SPEED_PID_KD);
    PID_Init(&s_line_pid, APP_LINE_PID_KP, APP_LINE_PID_KI, APP_LINE_PID_KD,
             -APP_LINE_TURN_LIMIT, APP_LINE_TURN_LIMIT);
    Control_LineTracker_Init(&s_line_pid);

    Telemetry_Init();
    Telemetry_BindChassis(&s_state_id,
                          &s_chassis.pose.x, &s_chassis.pose.y,
                          &s_chassis.pose.theta,
                          &s_chassis.target_speed_left,
                          &s_chassis.target_speed_right,
                          &s_chassis.measured_speed_left,
                          &s_chassis.measured_speed_right,
                          &s_line_turn);

    line_info = Control_LineTracker_GetInfo();
    Telemetry_BindLine(line_info->raw, BSP_ADC_CCD_COUNT,
                       &line_info->contrast, &line_info->strength,
                       &line_info->bias, &line_info->on_line);
    Telemetry_BindImu(&s_pitch_deg, &s_roll_deg, &s_yaw_deg,
                      &s_gx_dps, &s_gy_dps, &s_gz_dps,
                      &s_ax_g, &s_ay_g, &s_az_g, &s_imu_ready);

    set_state(APP_STATE_IDLE);

    (void)xTaskCreate(fast_task, "fast", 256u, 0, 4u, 0);
    (void)xTaskCreate(control_task, "ctrl", 384u, 0, 3u, 0);
    (void)xTaskCreate(telemetry_task, "telem", 384u, 0, 2u, 0);
}
