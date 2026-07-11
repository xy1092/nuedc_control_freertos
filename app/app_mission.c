#include "app_mission.h"
#include "app_config.h"
#include "app_main.h"
#include "app_shared.h"
#include "chassis.h"
#include "line_tracker.h"

static float clampf_local(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void App_Mission_Tick(uint32_t now_ms)
{
    AppContext_t *ctx = App_Shared_Get();
    (void)now_ms;

    if (ctx->state == APP_STATE_RUN) {
        float base = APP_BASE_SPEED_PULSE;
        float left_base = base * APP_LEFT_PULSES_PER_CM /
                          APP_RIGHT_PULSES_PER_CM;
        float turn = Control_LineTracker_Update();

        ctx->line_turn = clampf_local(turn, -APP_LINE_TURN_LIMIT,
                                      APP_LINE_TURN_LIMIT);
        Control_Chassis_SetTargetSpeeds(&ctx->chassis,
                                        left_base - ctx->line_turn,
                                        base + ctx->line_turn);
    } else {
        ctx->line_turn = 0.0f;
        Control_Chassis_SetTargetSpeeds(&ctx->chassis, 0.0f, 0.0f);
    }
}
