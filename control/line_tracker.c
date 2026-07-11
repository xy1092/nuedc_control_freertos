#include "line_tracker.h"
#include "bsp_adc.h"

#define TRACK_LOST_DIFF     0.18f

static PID_t *s_pid = 0;
static DevLineSensor *s_sensor = 0;
static ControlLineTrackInfo_t s_empty_info;

static const ControlLineTrackInfo_t *update_track_info(void)
{
    if (s_sensor == 0) {
        s_sensor = BSP_Adc_GetLineSensor();
    }

    if (s_sensor == 0) {
        return &s_empty_info;
    }

    return Dev_LineSensor_Update(s_sensor);
}

void Control_LineTracker_Init(PID_t *line_pid)
{
    s_pid = line_pid;
    s_sensor = BSP_Adc_GetLineSensor();
    if (s_sensor != 0) {
        Dev_LineSensor_Reset(s_sensor);
    }
}

float Control_LineTracker_Update(void)
{
    const ControlLineTrackInfo_t *info = update_track_info();

    if (!s_pid) return 0.0f;
    if (info == 0 || !info->on_line) {
        /* Safe forward-only seek diff (pulse/ms units). Caller in arc mode
         * overrides this, but other future callers must not pivot/reverse.
         */
        float hold = (info != 0) ? info->last_bias : 0.0f;
        if (hold > 0.0f) return  TRACK_LOST_DIFF;
        if (hold < 0.0f) return -TRACK_LOST_DIFF;
        return 0.0f;
    }

    return PID_Update(s_pid, 0.0f, info->bias);
}

const ControlLineTrackInfo_t *Control_LineTracker_GetInfo(void)
{
    const ControlLineTrackInfo_t *info;

    if (s_sensor == 0) {
        s_sensor = BSP_Adc_GetLineSensor();
    }

    info = Dev_LineSensor_GetFrame(s_sensor);
    return (info != 0) ? info : &s_empty_info;
}

uint8_t Control_LineTracker_IsOnLine(void)
{
    return Control_LineTracker_GetInfo()->on_line;
}

float Control_LineTracker_GetLastBias(void)
{
    return Control_LineTracker_GetInfo()->last_bias;
}
