#include "app_tasks.h"
#include "app_config.h"
#include "app_main.h"
#include "app_mission.h"
#include "app_shared.h"
#include "app_state.h"
#include "bsp_motor.h"
#include "chassis.h"
#include "telemetry.h"
#include "imu.h"
#include "fault_manager.h"
#include "health_monitor.h"
#include "watchdog.h"
#include "FreeRTOS.h"
#include "task.h"

#define FAST_STACK_WORDS       256u
#define CONTROL_STACK_WORDS    384u
#define TELEMETRY_STACK_WORDS  384u
#define IMU_STACK_WORDS        768u
#define HEALTH_STACK_WORDS     256u

extern volatile uint32_t g_tick_ms;

static StaticTask_t s_fast_tcb;
static StaticTask_t s_control_tcb;
static StaticTask_t s_telemetry_tcb;
static StaticTask_t s_imu_tcb;
static StaticTask_t s_health_tcb;
static StackType_t s_fast_stack[FAST_STACK_WORDS];
static StackType_t s_control_stack[CONTROL_STACK_WORDS];
static StackType_t s_telemetry_stack[TELEMETRY_STACK_WORDS];
static StackType_t s_imu_stack[IMU_STACK_WORDS];
static StackType_t s_health_stack[HEALTH_STACK_WORDS];

static void copy_imu_snapshot(AppContext_t *ctx)
{
    ImuSnapshot_t snapshot;

    IMU_GetSnapshot(&snapshot);
    ctx->pitch_deg = snapshot.pitch_deg;
    ctx->roll_deg = snapshot.roll_deg;
    ctx->yaw_deg = snapshot.yaw_deg;
    ctx->gx_dps = snapshot.gyro_dps[0];
    ctx->gy_dps = snapshot.gyro_dps[1];
    ctx->gz_dps = snapshot.gyro_dps[2];
    ctx->ax_g = snapshot.accel_g[0];
    ctx->ay_g = snapshot.accel_g[1];
    ctx->az_g = snapshot.accel_g[2];
    ctx->imu_ready = (snapshot.ready && snapshot.state == IMU_STATE_READY &&
                      snapshot.status == 0) ? 1u : 0u;
}

static void fast_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    AppContext_t *ctx = App_Shared_Get();
    (void)arg;

    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();

        g_tick_ms = now;
        Control_Chassis_PollFastInputs(&ctx->chassis);
        BSP_Motor_SafetyTick(now);
        copy_imu_snapshot(ctx);
        System_Health_Beat(HEALTH_TASK_FAST_IO, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_FAST_TASK_PERIOD_MS));
    }
}

static void control_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    AppContext_t *ctx = App_Shared_Get();
    (void)arg;

    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();

        g_tick_ms = now;
        App_State_Tick(now);
        App_Mission_Tick(now);
        Control_Chassis_Tick(&ctx->chassis,
                             (ctx->state == APP_STATE_RUN) ? 1u : 0u,
                             now);
        System_Health_Beat(HEALTH_TASK_CONTROL, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_CONTROL_TASK_PERIOD_MS));
    }
}

static void telemetry_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();
        TelemetryImuCalRequest_t imu_cal_request;

        g_tick_ms = now;
        Telemetry_Tick(now);
        imu_cal_request = Telemetry_ConsumeImuCalRequest();
        if (imu_cal_request == TELEMETRY_IMU_CAL_START) {
            IMU_RequestYawCalibration();
        } else if (imu_cal_request == TELEMETRY_IMU_CAL_CANCEL) {
            IMU_CancelYawCalibration();
        }
        System_Health_Beat(HEALTH_TASK_TELEMETRY, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_TELEM_TASK_PERIOD_MS));
    }
}

static void health_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    (void)arg;

    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();
        uint32_t stale_mask = 0u;

        if (System_Health_Check(now, &stale_mask) &&
            !System_Fault_IsActive()) {
            System_Watchdog_Feed();
        } else if (stale_mask != 0u) {
            System_Fault_Raise(SYSTEM_FAULT_TASK_STALE, stale_mask);
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(20u));
    }
}

static uint8_t create_tasks(void)
{
    if (xTaskCreateStatic(health_task, "health", HEALTH_STACK_WORDS, 0, 5u,
                          s_health_stack, &s_health_tcb) == 0) {
        return 0u;
    }
    if (xTaskCreateStatic(IMU_Service_Task, "imu", IMU_STACK_WORDS, 0, 4u,
                          s_imu_stack, &s_imu_tcb) == 0) {
        return 0u;
    }
    if (xTaskCreateStatic(fast_task, "fast", FAST_STACK_WORDS, 0, 4u,
                          s_fast_stack, &s_fast_tcb) == 0) {
        return 0u;
    }
    if (xTaskCreateStatic(control_task, "ctrl", CONTROL_STACK_WORDS, 0, 3u,
                          s_control_stack, &s_control_tcb) == 0) {
        return 0u;
    }
    if (xTaskCreateStatic(telemetry_task, "telem", TELEMETRY_STACK_WORDS, 0, 2u,
                          s_telemetry_stack, &s_telemetry_tcb) == 0) {
        return 0u;
    }
    return 1u;
}

void App_Tasks_Start(void)
{
    if (!create_tasks()) {
        System_Fault_Raise(SYSTEM_FAULT_HEAP, 0u);
        App_State_Set(APP_STATE_FAULT);
    }
}
