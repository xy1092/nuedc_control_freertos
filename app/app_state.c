#include "app_state.h"
#include "app_shared.h"
#include "bsp_motor.h"
#include "chassis.h"
#include "telemetry.h"

AppState_t App_GetState(void)
{
    return App_Shared_Get()->state;
}

void App_State_Set(AppState_t state)
{
    AppContext_t *ctx = App_Shared_Get();

    App_Shared_SetState(state);
    if (state != APP_STATE_RUN) {
        Control_Chassis_SetTargetSpeeds(&ctx->chassis, 0.0f, 0.0f);
        BSP_Motor_SetDuty(0, 0);
        BSP_Motor_SetDuty(1, 0);
    }
}

void App_State_Init(void)
{
    App_State_Set(APP_STATE_IDLE);
}

void App_State_Tick(uint32_t now_ms)
{
    AppContext_t *ctx = App_Shared_Get();
    TelemetryDutyRequest_t duty;

    if (Telemetry_ConsumeStopRequest()) {
        App_State_Set(APP_STATE_IDLE);
    }

    if (Telemetry_ConsumeStartRequest() && ctx->imu_ready) {
        ControlPose2D_t zero_pose = { 0.0f, 0.0f, 0.0f };
        Control_Chassis_ResetOdometry(&ctx->chassis, zero_pose);
        App_State_Set(APP_STATE_RUN);
    }

    if (Telemetry_ConsumeDutyRequest(&duty)) {
        App_State_Set(APP_STATE_IDLE);
        Control_Chassis_SetDebugDuty(&ctx->chassis, duty.left_pwm,
                                     duty.right_pwm, now_ms, duty.hold_ms);
    }
}
