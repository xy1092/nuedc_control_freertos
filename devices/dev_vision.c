#include "dev_vision.h"
#include <string.h>

static void clear_event(DevVisionEvent *event)
{
    if (event != 0) {
        memset(event, 0, sizeof(*event));
    }
}

static uint8_t dispatch_frame(DevVision *vision, uint32_t now_ms,
                              DevVisionEvent *event)
{
    clear_event(event);

    if (vision->frame_type == 0x01u && vision->len >= 7u) {
        vision->target.center_x = (int16_t)(vision->payload[0] |
                                  ((uint16_t)vision->payload[1] << 8));
        vision->target.center_y = (int16_t)(vision->payload[2] |
                                  ((uint16_t)vision->payload[3] << 8));
        vision->target.class_id = vision->payload[4];
        vision->target.area = (uint16_t)(vision->payload[5] |
                              ((uint16_t)vision->payload[6] << 8));
        vision->target.ts_ms = now_ms;
        vision->fresh_target = 1u;

        if (event != 0) {
            event->type = DEV_VISION_EVENT_TARGET;
            event->target = vision->target;
            event->frame_type = vision->frame_type;
        }
        return 1u;
    }

    if (vision->frame_type == 0x02u && vision->len >= 1u) {
        if (event != 0) {
            event->type = DEV_VISION_EVENT_COMMAND;
            event->command = vision->payload[0];
            event->frame_type = vision->frame_type;
        }
        return 1u;
    }

    return 0u;
}

void Dev_Vision_Init(DevVision *vision)
{
    if (vision == 0) {
        return;
    }

    memset(vision, 0, sizeof(*vision));
    vision->state = DEV_VISION_PS_SYNC1;
}

void Dev_Vision_ResetParser(DevVision *vision)
{
    if (vision == 0) {
        return;
    }

    vision->state = DEV_VISION_PS_SYNC1;
    vision->len = 0u;
    vision->frame_type = 0u;
    vision->index = 0u;
    vision->crc_calc = 0u;
}

uint8_t Dev_Vision_OnRxByte(DevVision *vision, uint8_t byte, uint32_t now_ms,
                            DevVisionEvent *event)
{
    if (vision == 0) {
        clear_event(event);
        return 0u;
    }

    clear_event(event);

    switch (vision->state) {
    case DEV_VISION_PS_SYNC1:
        if (byte == 0xAAu) {
            vision->state = DEV_VISION_PS_SYNC2;
        }
        break;

    case DEV_VISION_PS_SYNC2:
        vision->state = (byte == 0x55u) ? DEV_VISION_PS_LEN : DEV_VISION_PS_SYNC1;
        break;

    case DEV_VISION_PS_LEN:
        if (byte == 0u || byte > DEV_VISION_MAX_PAYLOAD) {
            vision->state = DEV_VISION_PS_SYNC1;
            break;
        }
        vision->len = byte;
        vision->crc_calc = byte;
        vision->state = DEV_VISION_PS_TYPE;
        break;

    case DEV_VISION_PS_TYPE:
        vision->frame_type = byte;
        vision->crc_calc ^= byte;
        vision->index = 0u;
        vision->state = DEV_VISION_PS_DATA;
        break;

    case DEV_VISION_PS_DATA:
        if (vision->index < vision->len) {
            vision->payload[vision->index++] = byte;
            vision->crc_calc ^= byte;
        }
        if (vision->index >= vision->len) {
            vision->state = DEV_VISION_PS_CRC;
        }
        break;

    case DEV_VISION_PS_CRC:
        vision->state = (byte == vision->crc_calc) ?
            DEV_VISION_PS_CR : DEV_VISION_PS_SYNC1;
        break;

    case DEV_VISION_PS_CR:
        vision->state = (byte == 0x0Du) ? DEV_VISION_PS_LF : DEV_VISION_PS_SYNC1;
        break;

    case DEV_VISION_PS_LF:
        vision->state = DEV_VISION_PS_SYNC1;
        if (byte == 0x0Au) {
            return dispatch_frame(vision, now_ms, event);
        }
        break;

    default:
        vision->state = DEV_VISION_PS_SYNC1;
        break;
    }

    return 0u;
}

uint8_t Dev_Vision_HasFreshTarget(DevVision *vision)
{
    if (vision != 0 && vision->fresh_target) {
        vision->fresh_target = 0u;
        return 1u;
    }
    return 0u;
}

DevVisionTarget Dev_Vision_GetTarget(const DevVision *vision)
{
    DevVisionTarget target;

    memset(&target, 0, sizeof(target));
    if (vision != 0) {
        target = vision->target;
    }
    return target;
}
