#include "bsp_encoder.h"
#include "pin_map.h"
#include <ti/driverlib/driverlib.h>

static uint8_t s_left_last_state;
static uint8_t s_right_last_state;
static uint8_t s_left_poll_state;
static uint8_t s_right_poll_state;
static volatile uint8_t s_left_seen_mask;
static volatile uint8_t s_right_seen_mask;
static volatile uint32_t s_left_change_count;
static volatile uint32_t s_right_change_count;
static volatile int64_t s_left_total;
static volatile int64_t s_right_total;
static int64_t s_left_reported;
static int64_t s_right_reported;

static uint8_t encoder_state(GPIO_Regs *port, uint32_t phase_a,
                             uint32_t phase_b)
{
    uint32_t pins = DL_GPIO_readPins(port, phase_a | phase_b);
    uint8_t a = ((pins & phase_a) != 0u) ? 1u : 0u;
    uint8_t b = ((pins & phase_b) != 0u) ? 1u : 0u;

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

void BSP_Encoder_Init(void)
{
    uint32_t primask = irq_lock();

    DL_GPIO_initDigitalInputFeatures(GPIO_QEI_LEFT_PHA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_QEI_LEFT_PHB_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_CAPTURE_RIGHT_C0_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_CAPTURE_RIGHT_C1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);

    s_left_last_state = encoder_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                      ENC_LEFT_B_PIN);
    s_right_last_state = encoder_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                       ENC_RIGHT_B_PIN);
    s_left_poll_state = s_left_last_state;
    s_right_poll_state = s_right_last_state;
    s_left_seen_mask = (uint8_t)(1u << s_left_poll_state);
    s_right_seen_mask = (uint8_t)(1u << s_right_poll_state);
    s_left_change_count = 0u;
    s_right_change_count = 0u;
    s_left_total = 0;
    s_left_reported = 0;
    s_right_total = 0;
    s_right_reported = 0;

    DL_GPIO_setLowerPinsPolarity(GPIOB,
        DL_GPIO_PIN_15_EDGE_RISE_FALL);
    DL_GPIO_setUpperPinsPolarity(GPIOB,
        DL_GPIO_PIN_16_EDGE_RISE_FALL |
        DL_GPIO_PIN_26_EDGE_RISE_FALL |
        DL_GPIO_PIN_27_EDGE_RISE_FALL);
    DL_GPIO_clearInterruptStatus(GPIOB,
        ENC_LEFT_A_PIN | ENC_LEFT_B_PIN |
        ENC_RIGHT_A_PIN | ENC_RIGHT_B_PIN);
    DL_GPIO_enableInterrupt(GPIOB,
        ENC_LEFT_A_PIN | ENC_LEFT_B_PIN |
        ENC_RIGHT_A_PIN | ENC_RIGHT_B_PIN);
    irq_unlock(primask);

    NVIC_ClearPendingIRQ(GPIOB_INT_IRQn);
    NVIC_EnableIRQ(GPIOB_INT_IRQn);
}

void BSP_Encoder_Poll(void)
{
    uint8_t left = encoder_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                 ENC_LEFT_B_PIN);
    uint8_t right = encoder_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                  ENC_RIGHT_B_PIN);

    s_left_seen_mask |= (uint8_t)(1u << left);
    s_right_seen_mask |= (uint8_t)(1u << right);
    if (left != s_left_poll_state) {
        s_left_change_count++;
        s_left_poll_state = left;
    }
    if (right != s_right_poll_state) {
        s_right_change_count++;
        s_right_poll_state = right;
    }
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
        s_left_last_state = encoder_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                          ENC_LEFT_B_PIN);
        s_left_poll_state = s_left_last_state;
        s_left_seen_mask = (uint8_t)(1u << s_left_poll_state);
        s_left_change_count = 0u;
        s_left_total = 0;
        s_left_reported = 0;
    } else {
        s_right_last_state = encoder_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                           ENC_RIGHT_B_PIN);
        s_right_poll_state = s_right_last_state;
        s_right_seen_mask = (uint8_t)(1u << s_right_poll_state);
        s_right_change_count = 0u;
        s_right_total = 0;
        s_right_reported = 0;
    }
    irq_unlock(primask);
}

uint8_t BSP_Encoder_GetRawState(uint8_t ch)
{
    return (ch == 0u) ? s_left_poll_state : s_right_poll_state;
}

uint8_t BSP_Encoder_GetSeenMask(uint8_t ch)
{
    return (ch == 0u) ? s_left_seen_mask : s_right_seen_mask;
}

uint32_t BSP_Encoder_GetChangeCount(uint8_t ch)
{
    return (ch == 0u) ? s_left_change_count : s_right_change_count;
}

void CAPTURE_RIGHT_INST_IRQHandler(void)
{
    (void)DL_TimerG_getPendingInterrupt(ENC_RIGHT_TIMER);
}

void GROUP1_IRQHandler(void)
{
    const uint32_t left_pins = ENC_LEFT_A_PIN | ENC_LEFT_B_PIN;
    const uint32_t right_pins = ENC_RIGHT_A_PIN | ENC_RIGHT_B_PIN;
    const uint32_t encoder_pins = left_pins | right_pins;
    uint32_t pending = DL_GPIO_getEnabledInterruptStatus(GPIOB,
                                                         encoder_pins);

    if ((pending & left_pins) != 0u) {
        uint8_t current = encoder_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                        ENC_LEFT_B_PIN);

        s_left_total += decode_step(s_left_last_state, current);
        s_left_last_state = current;
    }
    if ((pending & right_pins) != 0u) {
        uint8_t current = encoder_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                        ENC_RIGHT_B_PIN);

        s_right_total += decode_step(s_right_last_state, current);
        s_right_last_state = current;
    }
    DL_GPIO_clearInterruptStatus(GPIOB, pending);
}
