#include "ti_msp_dl_config.h"
#include "drv_gpio.h"
#include "drv_hbridge_motor.h"
#include "drv_uart.h"
#include "pin_map.h"
#include <stdint.h>

#define MOTOR_TEST_DUTY      300
#define LOOP_PERIOD_MS       10u
#define DEBOUNCE_SAMPLES     3u
#define HEARTBEAT_PERIOD_MS  500u

typedef enum {
    MOTOR_STATE_STOP = 0,
    MOTOR_STATE_LEFT,
    MOTOR_STATE_RIGHT,
    MOTOR_STATE_FORWARD,
} MotorTestState;

static const DrvHBridgeMotor s_left_motor = {
    { PWM_MOTOR_INST, PWM_CH_LEFT, 1000u, DRV_PWM_ACTIVE_LOW },
    { MOTOR_A_DIR_PORT, LEFT_AIN1_PIN, 0u },
    { MOTOR_A_DIR_PORT, LEFT_AIN2_PIN, 0u },
    -1,
};

static const DrvHBridgeMotor s_right_motor = {
    { PWM_MOTOR_INST, PWM_CH_RIGHT, 1000u, DRV_PWM_ACTIVE_LOW },
    { MOTOR_A_DIR_PORT, RIGHT_BIN1_PIN, 0u },
    { MOTOR_B_DIR_PORT, RIGHT_BIN2_PIN, 0u },
    -1,
};

static const DrvGpioPin s_left_button = {
    CALIBRATE_BUTTON_PORT, CALIBRATE_BUTTON_PIN, 1u,
};
static const DrvGpioPin s_right_button = {
    MODE_BUTTON_PORT, MODE_BUTTON_PIN, 1u,
};
static const DrvGpioPin s_forward_button = {
    START_STOP_BUTTON_PORT, START_STOP_BUTTON_PIN, 1u,
};
static const DrvUartPort s_uart = {
    UART_DEBUG_INST, UART_DEBUG_INST_INT_IRQN,
};

static void write_string(const char *text)
{
    uint16_t len = 0u;

    while (text[len] != '\0') {
        len++;
    }
    Drv_Uart_WriteBlocking(&s_uart, (const uint8_t *)text, len);
}

static MotorTestState read_button_state(void)
{
    uint8_t left = Drv_Gpio_ReadActive(&s_left_button);
    uint8_t right = Drv_Gpio_ReadActive(&s_right_button);
    uint8_t forward = Drv_Gpio_ReadActive(&s_forward_button);
    uint8_t pressed = (uint8_t)(left + right + forward);

    if (pressed != 1u) {
        return MOTOR_STATE_STOP;
    }
    if (left) {
        return MOTOR_STATE_LEFT;
    }
    if (right) {
        return MOTOR_STATE_RIGHT;
    }
    return MOTOR_STATE_FORWARD;
}

static void apply_state(MotorTestState state)
{
    Drv_HBridgeMotor_Coast(&s_left_motor);
    Drv_HBridgeMotor_Coast(&s_right_motor);

    switch (state) {
    case MOTOR_STATE_LEFT:
        Drv_HBridgeMotor_SetSignedDuty(&s_left_motor, MOTOR_TEST_DUTY);
        write_string("$MOTOR,LEFT,300,0\r\n");
        break;
    case MOTOR_STATE_RIGHT:
        Drv_HBridgeMotor_SetSignedDuty(&s_right_motor, MOTOR_TEST_DUTY);
        write_string("$MOTOR,RIGHT,0,300\r\n");
        break;
    case MOTOR_STATE_FORWARD:
        Drv_HBridgeMotor_SetSignedDuty(&s_left_motor, MOTOR_TEST_DUTY);
        Drv_HBridgeMotor_SetSignedDuty(&s_right_motor, MOTOR_TEST_DUTY);
        write_string("$MOTOR,FORWARD,300,300\r\n");
        break;
    case MOTOR_STATE_STOP:
    default:
        write_string("$MOTOR,STOP,0,0\r\n");
        break;
    }
}

int main(void)
{
    MotorTestState candidate = MOTOR_STATE_STOP;
    MotorTestState applied = (MotorTestState)0xFFu;
    uint16_t heartbeat_ms = 0u;
    uint8_t stable_samples = DEBOUNCE_SAMPLES;

    SYSCFG_DL_init();
    Drv_HBridgeMotor_Start(&s_left_motor);
    Drv_HBridgeMotor_Start(&s_right_motor);
    apply_state(MOTOR_STATE_STOP);
    applied = MOTOR_STATE_STOP;
    DL_WWDT_restart(WWDT0_INST);

    write_string("\r\nMOTOR BUTTON TEST v1\r\n");
    write_string("PA7=LEFT PB1=RIGHT PB21=FORWARD DUTY=300\r\n");

    for (;;) {
        MotorTestState sample;

        delay_cycles((CPUCLK_FREQ / 1000u) * LOOP_PERIOD_MS);
        DL_WWDT_restart(WWDT0_INST);
        heartbeat_ms = (uint16_t)(heartbeat_ms + LOOP_PERIOD_MS);
        sample = read_button_state();

        if (sample != candidate) {
            candidate = sample;
            stable_samples = 1u;
        } else if (stable_samples < DEBOUNCE_SAMPLES) {
            stable_samples++;
        }

        if (stable_samples >= DEBOUNCE_SAMPLES && applied != candidate) {
            apply_state(candidate);
            applied = candidate;
        }

        if (heartbeat_ms >= HEARTBEAT_PERIOD_MS) {
            heartbeat_ms = 0u;
            DL_GPIO_togglePins(STATUS_LED_PORT, STATUS_LED_PIN);
        }
    }
}
