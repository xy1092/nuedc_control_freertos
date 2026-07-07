#include "app_tasks.h"
#include "app_config.h"
#include "app_main.h"
#include "app_mission.h"
#include "app_shared.h"
#include "app_state.h"
#include "../bsp/bsp_imu.h"
#include "../middleware/control/chassis.h"
#include "../middleware/telemetry.h"
#include "FreeRTOS.h"
#include "task.h"

extern volatile uint32_t g_tick_ms;

static void fast_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    AppContext_t *ctx = App_Shared_Get();
    (void)arg;

    for (;;) {
        g_tick_ms = (uint32_t)xTaskGetTickCount();
        Control_Chassis_PollFastInputs(&ctx->chassis);
        BSP_IMU_Update(&ctx->pitch_deg, &ctx->roll_deg, &ctx->yaw_deg);
        BSP_IMU_GetGyro(&ctx->gx_dps, &ctx->gy_dps, &ctx->gz_dps);
        BSP_IMU_GetAccel(&ctx->ax_g, &ctx->ay_g, &ctx->az_g);
        ctx->imu_ready = BSP_IMU_IsReady();
        vTaskDelayUntil(&last, pdMS_TO_TICKS(APP_FAST_TASK_PERIOD_MS));
    }
}

static void control_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    AppContext_t *ctx = App_Shared_Get();
    (void)arg;

    for (;;) {
        uint32_t now_ms = (uint32_t)xTaskGetTickCount();

        g_tick_ms = now_ms;
        App_State_Tick(now_ms);
        App_Mission_Tick(now_ms);
        Control_Chassis_Tick(&ctx->chassis,
                             (ctx->state == APP_STATE_RUN) ? 1u : 0u,
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

void App_Tasks_Start(void)
{
    if (xTaskCreate(fast_task, "fast", 256u, 0, 4u, 0) != pdPASS) {
        App_State_Set(APP_STATE_FAULT);
    }
    if (xTaskCreate(control_task, "ctrl", 384u, 0, 3u, 0) != pdPASS) {
        App_State_Set(APP_STATE_FAULT);
    }
    if (xTaskCreate(telemetry_task, "telem", 384u, 0, 2u, 0) != pdPASS) {
        App_State_Set(APP_STATE_FAULT);
    }
}
