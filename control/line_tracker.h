#ifndef MIDDLEWARE_CONTROL_LINE_TRACKER_H
#define MIDDLEWARE_CONTROL_LINE_TRACKER_H

#include <stdint.h>
#include "dev_line_sensor.h"
#include "pid.h"

typedef DevLineSensorFrame ControlLineTrackInfo_t;

void                          Control_LineTracker_Init(PID_t *line_pid);
float                         Control_LineTracker_Update(void);
const ControlLineTrackInfo_t *Control_LineTracker_GetInfo(void);
uint8_t                       Control_LineTracker_IsOnLine(void);
float                         Control_LineTracker_GetLastBias(void);

#endif
