#include "app_main.h"
#include "app_config.h"
#include "app_shared.h"
#include "app_state.h"
#include "app_tasks.h"
#include "bsp_adc.h"
#include "bsp_motor.h"
#include "bsp_uart.h"
#include "chassis.h"
#include "line_tracker.h"
#include "pid.h"
#include "telemetry.h"
#include "imu.h"
#include "fault_manager.h"
#include "health_monitor.h"

static const ControlChassisConfig_t k_chassis_config = {
    .left_pulses_per_cm = APP_LEFT_PULSES_PER_CM,
    .right_pulses_per_cm = APP_RIGHT_PULSES_PER_CM,
    .wheel_base_cm = APP_WHEEL_BASE_CM,
    .open_loop_cm_per_pwm_ms = APP_OPEN_LOOP_CM_PER_PWM_MS,
    .speed_out_limit = APP_SPEED_OUT_LIMIT,
    .forward_min_pulse = APP_FORWARD_MIN_PULSE,
};

void App_Init(void)
{
    AppContext_t *ctx = App_Shared_Get();
    const ControlLineTrackInfo_t *line_info;

    BSP_Motor_Init();
    BSP_Uart_Init();
    BSP_Adc_Init();
    IMU_Service_Init();
    System_Fault_Init();
    System_Health_Init(0u);

    Control_Chassis_Init(&ctx->chassis, &k_chassis_config,
                         APP_SPEED_PID_KP, APP_SPEED_PID_KI,
                         APP_SPEED_PID_KD);
    PID_Init(&ctx->line_pid, APP_LINE_PID_KP, APP_LINE_PID_KI, APP_LINE_PID_KD,
             -APP_LINE_TURN_LIMIT, APP_LINE_TURN_LIMIT);
    Control_LineTracker_Init(&ctx->line_pid);

    Telemetry_Init();
    Telemetry_BindChassis(&ctx->state_id,
                          &ctx->chassis.pose.x, &ctx->chassis.pose.y,
                          &ctx->chassis.pose.theta,
                          &ctx->chassis.target_speed_left,
                          &ctx->chassis.target_speed_right,
                          &ctx->chassis.measured_speed_left,
                          &ctx->chassis.measured_speed_right,
                          &ctx->line_turn);

    line_info = Control_LineTracker_GetInfo();
    Telemetry_BindLine(line_info->raw, BSP_ADC_CCD_COUNT,
                       &line_info->contrast, &line_info->strength,
                       &line_info->bias, &line_info->on_line);
    Telemetry_BindImu(&ctx->pitch_deg, &ctx->roll_deg, &ctx->yaw_deg,
                      &ctx->gx_dps, &ctx->gy_dps, &ctx->gz_dps,
                      &ctx->ax_g, &ctx->ay_g, &ctx->az_g, &ctx->imu_ready);

    App_State_Init();
    App_Tasks_Start();
}
