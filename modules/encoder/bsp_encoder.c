#include "bsp_encoder.h"
#include "pin_map.h"
#include <ti/driverlib/driverlib.h>

static uint16_t s_left_last_count;
static int64_t s_left_total;
static int64_t s_left_reported;
static volatile uint8_t s_right_last_state;
static volatile int64_t s_right_total;
static int64_t s_right_reported;

static uint8_t right_state(void)
{
    uint8_t a = (DL_GPIO_readPins(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN) != 0u) ? 1u : 0u;
    uint8_t b = (DL_GPIO_readPins(ENC_RIGHT_PORT, ENC_RIGHT_B_PIN) != 0u) ? 1u : 0u;

    return (uint8_t)((a << 1) | b);
}

static int8_t decode_step(uint8_t last, uint8_t current)
{
    static const int8_t table[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0,
    };

    return table[((last & 0x03u) << 2) | (current & 0x03u)];
}

static uint32_t irq_lock(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static void irq_unlock(uint32_t primask)
{
    __set_PRIMASK(primask);
}

static void poll_left(void)
{
    uint16_t current = (uint16_t)DL_TimerG_getTimerCount(ENC_LEFT_TIMER);
    int16_t step = (int16_t)(current - s_left_last_count);

    s_left_last_count = current;
    s_left_total += step;
}

void BSP_Encoder_Init(void)
{
    uint32_t primask = irq_lock();

    s_left_last_count = (uint16_t)DL_TimerG_getTimerCount(ENC_LEFT_TIMER);
    s_left_total = 0;
    s_left_reported = 0;
    s_right_last_state = right_state();
    s_right_total = 0;
    s_right_reported = 0;
    irq_unlock(primask);

    DL_TimerG_startCounter(ENC_LEFT_TIMER);
    NVIC_ClearPendingIRQ(ENC_RIGHT_TIMER_IRQN);
    NVIC_EnableIRQ(ENC_RIGHT_TIMER_IRQN);
}

void BSP_Encoder_Poll(void)
{
    poll_left();
}

int32_t BSP_Encoder_GetDelta(uint8_t ch)
{
    int64_t total;
    int64_t delta;
    uint32_t primask = irq_lock();

    if (ch == 0u) {
        total = s_left_total;
        delta = total - s_left_reported;
        s_left_reported = total;
    } else {
        total = s_right_total;
        delta = total - s_right_reported;
        s_right_reported = total;
    }
    irq_unlock(primask);

    return (int32_t)delta;
}

int64_t BSP_Encoder_GetTotal(uint8_t ch)
{
    int64_t total;
    uint32_t primask = irq_lock();

    total = (ch == 0u) ? s_left_total : s_right_total;
    irq_unlock(primask);
    return total;
}

void BSP_Encoder_Reset(uint8_t ch)
{
    uint32_t primask = irq_lock();

    if (ch == 0u) {
        s_left_last_count = (uint16_t)DL_TimerG_getTimerCount(ENC_LEFT_TIMER);
        s_left_total = 0;
        s_left_reported = 0;
    } else {
        s_right_last_state = right_state();
        s_right_total = 0;
        s_right_reported = 0;
    }
    irq_unlock(primask);
}

void CAPTURE_RIGHT_INST_IRQHandler(void)
{
    DL_TIMER_IIDX pending = DL_TimerG_getPendingInterrupt(ENC_RIGHT_TIMER);

    if (pending == DL_TIMER_IIDX_CC0_DN || pending == DL_TIMER_IIDX_CC0_UP ||
        pending == DL_TIMER_IIDX_CC1_DN || pending == DL_TIMER_IIDX_CC1_UP) {
        uint8_t current = right_state();
        s_right_total += decode_step(s_right_last_state, current);
        s_right_last_state = current;
    }
}
