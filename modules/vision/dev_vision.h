#ifndef DEV_VISION_H
#define DEV_VISION_H

#include <stdint.h>

#define DEV_VISION_MAX_PAYLOAD 32u

typedef struct {
    int16_t center_x;
    int16_t center_y;
    uint8_t class_id;
    uint16_t area;
    uint32_t ts_ms;
} DevVisionTarget;

typedef enum {
    DEV_VISION_EVENT_NONE = 0,
    DEV_VISION_EVENT_TARGET,
    DEV_VISION_EVENT_COMMAND,
} DevVisionEventType;

typedef struct {
    DevVisionEventType type;
    DevVisionTarget target;
    uint8_t command;
    uint8_t frame_type;
} DevVisionEvent;

typedef enum {
    DEV_VISION_PS_SYNC1 = 0,
    DEV_VISION_PS_SYNC2,
    DEV_VISION_PS_LEN,
    DEV_VISION_PS_TYPE,
    DEV_VISION_PS_DATA,
    DEV_VISION_PS_CRC,
    DEV_VISION_PS_CR,
    DEV_VISION_PS_LF,
} DevVisionParseState;

typedef struct {
    DevVisionParseState state;
    uint8_t len;
    uint8_t frame_type;
    uint8_t index;
    uint8_t crc_calc;
    uint8_t payload[DEV_VISION_MAX_PAYLOAD];
    DevVisionTarget target;
    uint8_t fresh_target;
} DevVision;

void Dev_Vision_Init(DevVision *vision);
void Dev_Vision_ResetParser(DevVision *vision);
uint8_t Dev_Vision_OnRxByte(DevVision *vision, uint8_t byte, uint32_t now_ms,
                            DevVisionEvent *event);
uint8_t Dev_Vision_HasFreshTarget(DevVision *vision);
DevVisionTarget Dev_Vision_GetTarget(const DevVision *vision);

#endif
