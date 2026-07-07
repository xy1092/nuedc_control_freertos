#include "rtt_log.h"

#define RTT_UP_BUFFER_SIZE    4096u
#define RTT_DOWN_BUFFER_SIZE  16u

typedef struct {
    const char *name;
    char *buffer;
    uint32_t size;
    uint32_t wr_off;
    volatile uint32_t rd_off;
    uint32_t flags;
} RTTBufferUp_t;

typedef struct {
    const char *name;
    char *buffer;
    uint32_t size;
    volatile uint32_t wr_off;
    uint32_t rd_off;
    uint32_t flags;
} RTTBufferDown_t;

typedef struct {
    char id[16];
    int32_t max_up_buffers;
    int32_t max_down_buffers;
    RTTBufferUp_t up[1];
    RTTBufferDown_t down[1];
} RTTControlBlock_t;

static char s_up_buffer[RTT_UP_BUFFER_SIZE];
static char s_down_buffer[RTT_DOWN_BUFFER_SIZE];

RTTControlBlock_t _SEGGER_RTT = {
    "SEGGER RTT",
    1,
    1,
    {
        {
            "Terminal",
            s_up_buffer,
            RTT_UP_BUFFER_SIZE,
            0,
            0,
            0,
        },
    },
    {
        {
            "Command",
            s_down_buffer,
            RTT_DOWN_BUFFER_SIZE,
            0,
            0,
            0,
        },
    },
};

static uint32_t cstr_len(const char *s)
{
    uint32_t n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

static uint32_t space_available(const RTTBufferUp_t *buf)
{
    uint32_t wr = buf->wr_off;
    uint32_t rd = buf->rd_off;

    if (rd > wr) return rd - wr - 1u;
    return buf->size - wr + rd - 1u;
}

void RTT_Log_Write(const char *data, uint32_t len)
{
    RTTBufferUp_t *buf = &_SEGGER_RTT.up[0];
    uint32_t wr;

    if (!data || len == 0u) return;
    if (len >= buf->size) return;
    if (space_available(buf) < len) return;

    wr = buf->wr_off;
    for (uint32_t i = 0; i < len; ++i) {
        buf->buffer[wr++] = data[i];
        if (wr >= buf->size) wr = 0u;
    }
    buf->wr_off = wr;
}

void RTT_Log_WriteString(const char *s)
{
    if (!s) return;
    RTT_Log_Write(s, cstr_len(s));
}
