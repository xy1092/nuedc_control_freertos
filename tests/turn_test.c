#include "ti_msp_dl_config.h"
#ifdef NUEDC_ENCODER_CALIBRATION
#include "bsp_encoder.h"
#endif
#include "bsp_motor.h"
#include "drv_gpio.h"
#include "drv_uart.h"
#include "fault_manager.h"
#include "health_monitor.h"
#include "imu.h"
#include "pin_map.h"
#include "ssd1306_mspm0.h"
#include "watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef NUEDC_ENCODER_CALIBRATION
#define TURN_ANGLE_DEG             360.0f
#define TURN_PRECISION_DEG         0.5f
#define TURN_FINE_MAX_ATTEMPTS     12u
#define TURN_TIMEOUT_MS            20000u
#else
#define TURN_ANGLE_DEG             90.0f
#define TURN_PRECISION_DEG         0.2f
#define TURN_FINE_MAX_ATTEMPTS     20u
#define TURN_TIMEOUT_MS            12000u
#endif
#define TURN_PWM_MAX               300
#define TURN_PWM_MIN               220
#define TURN_KP                    4.0f
#define TURN_APPROACH_WINDOW_DEG   20.0f
#define TURN_PWM_APPROACH_MIN      160
#define TURN_FINE_ENTRY_DEG        3.0f
#define TURN_SETTLE_RATE_DPS       1.0f
#define TURN_SETTLE_SAMPLES        15u
#define TURN_BRAKE_MIN_MS          120u
#define TURN_FINE_PWM_INITIAL      180
#define TURN_FINE_PWM_MIN          140
#define TURN_FINE_PWM_MAX          260
#define TURN_FINE_PWM_STEP         20
#define TURN_FINE_PULSE_MS         20u
#define TURN_FINE_PROGRESS_MIN_DEG 0.05f
#define TURN_DIRECTION_CHECK_MS    600u
#define TURN_DIRECTION_PROGRESS_DEG 2.0f
#define SEQUENCE_PAUSE_MS          700u
#define CONTROL_PERIOD_MS          10u
#define UI_PERIOD_MS               100u
#define DEBOUNCE_SAMPLES           3u
#define IMU_STACK_WORDS            768u
#define CONTROL_STACK_WORDS        512u
#define UI_STACK_WORDS             512u
#define HEALTH_STACK_WORDS         256u

typedef enum {
    BUTTON_NONE = 0,
    BUTTON_CW,
    BUTTON_CCW,
    BUTTON_SEQUENCE,
} ButtonCommand;

typedef enum {
    TURN_STATE_WAIT_IMU = 0,
    TURN_STATE_IDLE,
    TURN_STATE_CW,
    TURN_STATE_CCW,
    TURN_STATE_SEQUENCE_CW,
    TURN_STATE_SEQUENCE_PAUSE,
    TURN_STATE_SEQUENCE_CCW,
    TURN_STATE_DONE,
    TURN_STATE_FAULT,
} TurnState;

typedef enum {
    TURN_PHASE_COARSE = 0,
    TURN_PHASE_BRAKE,
    TURN_PHASE_FINE_PULSE,
} TurnPhase;

typedef struct {
    TurnState state;
    float yaw_deg;
    float target_deg;
    float error_deg;
    int16_t pwm;
    int status;
    TurnPhase phase;
    uint8_t fine_attempts;
#ifdef NUEDC_ENCODER_CALIBRATION
    int64_t left_total;
    int64_t right_total;
    int64_t cw_left_result;
    int64_t cw_right_result;
    int64_t ccw_left_result;
    int64_t ccw_right_result;
    uint8_t left_raw;
    uint8_t right_raw;
    uint8_t left_seen_mask;
    uint8_t right_seen_mask;
#endif
} TurnSnapshot;

volatile uint32_t g_tick_ms;

static TurnSnapshot s_turn_snapshot;
#ifdef NUEDC_ENCODER_CALIBRATION
static int64_t s_cw_left_result;
static int64_t s_cw_right_result;
static int64_t s_ccw_left_result;
static int64_t s_ccw_right_result;
static uint8_t s_left_raw;
static uint8_t s_right_raw;
static uint8_t s_left_seen_mask;
static uint8_t s_right_seen_mask;
static uint8_t s_left_last_raw;
static uint8_t s_right_last_raw;
static int64_t s_left_software_total;
static int64_t s_right_software_total;
#endif
static bool s_oled_ok;
static uint8_t s_oled_address = SSD1306_I2C_ADDRESS_DEFAULT;
static const DrvGpioPin s_cw_button = {
    CALIBRATE_BUTTON_PORT, CALIBRATE_BUTTON_PIN, 1u,
};
static const DrvGpioPin s_ccw_button = {
    MODE_BUTTON_PORT, MODE_BUTTON_PIN, 1u,
};
static const DrvGpioPin s_sequence_button = {
    START_STOP_BUTTON_PORT, START_STOP_BUTTON_PIN, 1u,
};
static const DrvUartPort s_uart = {
    UART_DEBUG_INST, UART_DEBUG_INST_INT_IRQN,
};

static StaticTask_t s_imu_tcb;
static StaticTask_t s_control_tcb;
static StaticTask_t s_ui_tcb;
static StaticTask_t s_health_tcb;
static StaticTask_t s_idle_tcb;
static StackType_t s_imu_stack[IMU_STACK_WORDS];
static StackType_t s_control_stack[CONTROL_STACK_WORDS];
static StackType_t s_ui_stack[UI_STACK_WORDS];
static StackType_t s_health_stack[HEALTH_STACK_WORDS];
static StackType_t s_idle_stack[configMINIMAL_STACK_SIZE];

static float abs_float(float value)
{
    return (value < 0.0f) ? -value : value;
}

static int32_t to_tenths(float value)
{
    float scaled = value * 10.0f;

    return (int32_t)((scaled >= 0.0f) ? scaled + 0.5f : scaled - 0.5f);
}

static int32_t to_millidegrees(float value)
{
    float scaled = value * 1000.0f;

    return (int32_t)((scaled >= 0.0f) ? scaled + 0.5f : scaled - 0.5f);
}

static void uart_printf(const char *format, ...)
{
    char text[192];
    va_list args;
    int length;

    va_start(args, format);
    length = vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    if (length <= 0) {
        return;
    }
    if (length >= (int)sizeof(text)) {
        length = (int)sizeof(text) - 1;
    }
    Drv_Uart_WriteBlocking(&s_uart, (const uint8_t *)text, (uint16_t)length);
}

static void publish_turn_snapshot(TurnState state, const ImuSnapshot_t *imu,
                                  float target, float error, int16_t pwm,
                                  int status, TurnPhase phase,
                                  uint8_t fine_attempts)
{
    taskENTER_CRITICAL();
    s_turn_snapshot.state = state;
    s_turn_snapshot.yaw_deg = (imu != 0) ? imu->yaw_deg : 0.0f;
    s_turn_snapshot.target_deg = target;
    s_turn_snapshot.error_deg = error;
    s_turn_snapshot.pwm = pwm;
    s_turn_snapshot.status = status;
    s_turn_snapshot.phase = phase;
    s_turn_snapshot.fine_attempts = fine_attempts;
#ifdef NUEDC_ENCODER_CALIBRATION
    s_turn_snapshot.left_total = s_left_software_total;
    s_turn_snapshot.right_total = s_right_software_total;
    s_turn_snapshot.cw_left_result = s_cw_left_result;
    s_turn_snapshot.cw_right_result = s_cw_right_result;
    s_turn_snapshot.ccw_left_result = s_ccw_left_result;
    s_turn_snapshot.ccw_right_result = s_ccw_right_result;
    s_turn_snapshot.left_raw = s_left_raw;
    s_turn_snapshot.right_raw = s_right_raw;
    s_turn_snapshot.left_seen_mask = s_left_seen_mask;
    s_turn_snapshot.right_seen_mask = s_right_seen_mask;
#endif
    taskEXIT_CRITICAL();
}

static void read_turn_snapshot(TurnSnapshot *snapshot)
{
    taskENTER_CRITICAL();
    *snapshot = s_turn_snapshot;
    taskEXIT_CRITICAL();
}

static ButtonCommand read_buttons(void)
{
    uint8_t cw = Drv_Gpio_ReadActive(&s_cw_button);
    uint8_t ccw = Drv_Gpio_ReadActive(&s_ccw_button);
    uint8_t sequence = Drv_Gpio_ReadActive(&s_sequence_button);

    if ((uint8_t)(cw + ccw + sequence) != 1u) {
        return BUTTON_NONE;
    }
    if (cw) {
        return BUTTON_CW;
    }
    if (ccw) {
        return BUTTON_CCW;
    }
    return BUTTON_SEQUENCE;
}

#ifdef NUEDC_ENCODER_CALIBRATION
static uint8_t encoder_raw_state(GPIO_Regs *port, uint32_t phase_a,
                                 uint32_t phase_b)
{
    uint32_t pins = DL_GPIO_readPins(port, phase_a | phase_b);
    uint8_t a = ((pins & phase_a) != 0u) ? 1u : 0u;
    uint8_t b = ((pins & phase_b) != 0u) ? 1u : 0u;

    return (uint8_t)((a << 1u) | b);
}

static int8_t decode_encoder_step(uint8_t previous, uint8_t current)
{
    static const int8_t table[16] = {
         0,  1, -1,  0,
        -1,  0,  0,  1,
         1,  0,  0, -1,
         0, -1,  1,  0,
    };

    return table[((previous & 0x03u) << 2u) | (current & 0x03u)];
}

static void sample_encoder_raw_states(void)
{
    s_left_raw = encoder_raw_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                   ENC_LEFT_B_PIN);
    s_right_raw = encoder_raw_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                    ENC_RIGHT_B_PIN);
    s_left_software_total += decode_encoder_step(s_left_last_raw, s_left_raw);
    s_right_software_total += decode_encoder_step(s_right_last_raw,
                                                   s_right_raw);
    s_left_last_raw = s_left_raw;
    s_right_last_raw = s_right_raw;
    s_left_seen_mask |= (uint8_t)(1u << s_left_raw);
    s_right_seen_mask |= (uint8_t)(1u << s_right_raw);
}

static void encoder_diagnostic_reset(void)
{
    s_left_raw = encoder_raw_state(ENC_LEFT_PORT, ENC_LEFT_A_PIN,
                                   ENC_LEFT_B_PIN);
    s_right_raw = encoder_raw_state(ENC_RIGHT_PORT, ENC_RIGHT_A_PIN,
                                    ENC_RIGHT_B_PIN);
    s_left_last_raw = s_left_raw;
    s_right_last_raw = s_right_raw;
    s_left_software_total = 0;
    s_right_software_total = 0;
    s_left_seen_mask = (uint8_t)(1u << s_left_raw);
    s_right_seen_mask = (uint8_t)(1u << s_right_raw);
}

static void encoder_gpio_diagnostic_init(void)
{
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
    encoder_diagnostic_reset();
}
#endif

static void stop_motors(void)
{
    BSP_Motor_SetSignedDuty(0u, 0);
    BSP_Motor_SetSignedDuty(1u, 0);
}

static void brake_motors(void)
{
    BSP_Motor_Brake(0u);
    BSP_Motor_Brake(1u);
}

static int16_t turn_pwm(float error_deg)
{
    float error_magnitude = abs_float(error_deg);
    float magnitude;

    if (error_magnitude < TURN_APPROACH_WINDOW_DEG) {
        float span = (float)(TURN_PWM_MIN - TURN_PWM_APPROACH_MIN);

        magnitude = (float)TURN_PWM_APPROACH_MIN +
                    span * error_magnitude / TURN_APPROACH_WINDOW_DEG;
    } else {
        magnitude = error_magnitude * TURN_KP;
    }

    if (error_magnitude >= TURN_APPROACH_WINDOW_DEG &&
        magnitude < (float)TURN_PWM_MIN) {
        magnitude = (float)TURN_PWM_MIN;
    }
    if (magnitude > (float)TURN_PWM_MAX) {
        magnitude = (float)TURN_PWM_MAX;
    }
    return (int16_t)magnitude;
}

static int16_t clamp_fine_pwm(int16_t pwm)
{
    if (pwm < TURN_FINE_PWM_MIN) {
        return TURN_FINE_PWM_MIN;
    }
    if (pwm > TURN_FINE_PWM_MAX) {
        return TURN_FINE_PWM_MAX;
    }
    return pwm;
}

static void apply_turn(float error_deg, int16_t pwm)
{
    if (error_deg > 0.0f) {
        /* Positive yaw is counterclockwise: left reverse, right forward. */
        BSP_Motor_SetSignedDuty(0u, (int16_t)-pwm);
        BSP_Motor_SetSignedDuty(1u, pwm);
    } else {
        BSP_Motor_SetSignedDuty(0u, pwm);
        BSP_Motor_SetSignedDuty(1u, (int16_t)-pwm);
    }
}

static const char *state_text(TurnState state)
{
    switch (state) {
    case TURN_STATE_IDLE: return "IDLE";
#ifdef NUEDC_ENCODER_CALIBRATION
    case TURN_STATE_CW: return "CW 360";
    case TURN_STATE_CCW: return "CCW 360";
    case TURN_STATE_SEQUENCE_CW: return "SEQ CW 360";
    case TURN_STATE_SEQUENCE_PAUSE: return "SEQ PAUSE";
    case TURN_STATE_SEQUENCE_CCW: return "SEQ CCW 360";
#else
    case TURN_STATE_CW: return "CW 90";
    case TURN_STATE_CCW: return "CCW 90";
    case TURN_STATE_SEQUENCE_CW: return "SEQ CW 90";
    case TURN_STATE_SEQUENCE_PAUSE: return "SEQ PAUSE";
    case TURN_STATE_SEQUENCE_CCW: return "SEQ CCW 90";
#endif
    case TURN_STATE_DONE: return "DONE";
    case TURN_STATE_FAULT: return "FAULT";
    case TURN_STATE_WAIT_IMU:
    default: return "WAIT IMU";
    }
}

static const char *phase_text(TurnPhase phase)
{
    switch (phase) {
    case TURN_PHASE_BRAKE: return "BRK";
    case TURN_PHASE_FINE_PULSE: return "FINE";
    case TURN_PHASE_COARSE:
    default: return "COARSE";
    }
}

static bool state_is_turning(TurnState state)
{
    return state == TURN_STATE_CW || state == TURN_STATE_CCW ||
           state == TURN_STATE_SEQUENCE_CW ||
           state == TURN_STATE_SEQUENCE_CCW;
}

static void control_task(void *argument)
{
    TickType_t last = xTaskGetTickCount();
    ButtonCommand candidate = BUTTON_NONE;
    ButtonCommand stable = BUTTON_NONE;
    uint8_t stable_samples = DEBOUNCE_SAMPLES;
    TurnState state = TURN_STATE_WAIT_IMU;
    TurnPhase phase = TURN_PHASE_COARSE;
    float target_deg = 0.0f;
    float turn_origin_deg = 0.0f;
    int8_t expected_direction = 0;
    uint16_t settle_samples = 0u;
    uint32_t phase_started_ms = 0u;
    uint32_t segment_started_ms = 0u;
    int fault_status = 0;
    int16_t fine_pwm = TURN_FINE_PWM_INITIAL;
    uint8_t fine_attempts = 0u;
    int8_t fine_direction = 0;
    float fine_start_yaw_deg = 0.0f;
    float fine_error_before_deg = 0.0f;

    (void)argument;
    for (;;) {
        ImuSnapshot_t imu;
        ButtonCommand sample;
        ButtonCommand pressed = BUTTON_NONE;
        uint32_t now = (uint32_t)xTaskGetTickCount();
        float error_deg = 0.0f;
        int16_t pwm = 0;
        int status = fault_status;

        g_tick_ms = now;
#ifdef NUEDC_ENCODER_CALIBRATION
        sample_encoder_raw_states();
#endif
        IMU_GetSnapshot(&imu);
        sample = read_buttons();
        if (sample != candidate) {
            candidate = sample;
            stable_samples = 1u;
        } else if (stable_samples < DEBOUNCE_SAMPLES) {
            stable_samples++;
        }
        if (stable_samples >= DEBOUNCE_SAMPLES && stable != candidate) {
            stable = candidate;
            if (stable != BUTTON_NONE) {
                pressed = stable;
            }
        }

        if (System_Fault_IsActive()) {
            state = TURN_STATE_FAULT;
            if (fault_status == 0) {
                fault_status = -5;
            }
            status = fault_status;
        } else if (imu.state == IMU_STATE_FAULT || imu.status != 0) {
            state = TURN_STATE_FAULT;
            fault_status = (imu.status != 0) ? imu.status : -1;
            status = fault_status;
        } else if (imu.state != IMU_STATE_READY || !imu.ready) {
            if (state_is_turning(state)) {
                state = TURN_STATE_FAULT;
                fault_status = -4;
                status = fault_status;
            } else if (state != TURN_STATE_FAULT) {
                state = TURN_STATE_WAIT_IMU;
            }
        } else if (state == TURN_STATE_WAIT_IMU) {
            state = TURN_STATE_IDLE;
        }

        if ((state == TURN_STATE_IDLE || state == TURN_STATE_DONE) &&
            pressed != BUTTON_NONE && imu.state == IMU_STATE_READY &&
            imu.ready && imu.status == 0) {
            turn_origin_deg = imu.yaw_deg;
            settle_samples = 0u;
            phase_started_ms = now;
            segment_started_ms = now;
            phase = TURN_PHASE_COARSE;
            fine_pwm = TURN_FINE_PWM_INITIAL;
            fine_attempts = 0u;
#ifdef NUEDC_ENCODER_CALIBRATION
            encoder_diagnostic_reset();
            s_cw_left_result = 0;
            s_cw_right_result = 0;
            s_ccw_left_result = 0;
            s_ccw_right_result = 0;
            s_left_seen_mask = 0u;
            s_right_seen_mask = 0u;
            sample_encoder_raw_states();
#endif
            if (pressed == BUTTON_CW) {
                state = TURN_STATE_CW;
                expected_direction = -1;
                target_deg = turn_origin_deg - TURN_ANGLE_DEG;
            } else if (pressed == BUTTON_CCW) {
                state = TURN_STATE_CCW;
                expected_direction = 1;
                target_deg = turn_origin_deg + TURN_ANGLE_DEG;
            } else {
                state = TURN_STATE_SEQUENCE_CW;
                expected_direction = -1;
                target_deg = turn_origin_deg - TURN_ANGLE_DEG;
            }
        }

        if (state_is_turning(state)) {
            float progress = (imu.yaw_deg - turn_origin_deg) *
                             (float)expected_direction;

            error_deg = target_deg - imu.yaw_deg;
            if ((uint32_t)(now - segment_started_ms) > TURN_TIMEOUT_MS) {
                state = TURN_STATE_FAULT;
                fault_status = -2;
                status = fault_status;
            } else if (phase == TURN_PHASE_COARSE &&
                       (uint32_t)(now - segment_started_ms) >=
                           TURN_DIRECTION_CHECK_MS &&
                       progress < TURN_DIRECTION_PROGRESS_DEG) {
                state = TURN_STATE_FAULT;
                fault_status = -3;
                status = fault_status;
            } else if (phase == TURN_PHASE_COARSE) {
                if (abs_float(error_deg) <= TURN_FINE_ENTRY_DEG ||
                    (error_deg * (float)expected_direction) <= 0.0f) {
                    phase = TURN_PHASE_BRAKE;
                    phase_started_ms = now;
                    settle_samples = 0u;
                    brake_motors();
                } else {
                    pwm = turn_pwm(error_deg);
                    apply_turn(error_deg, pwm);
                }
            } else if (phase == TURN_PHASE_FINE_PULSE) {
                bool crossed_target =
                    (error_deg * (float)fine_direction) <= 0.0f;

                if (crossed_target ||
                    (uint32_t)(now - phase_started_ms) >=
                        TURN_FINE_PULSE_MS) {
                    phase = TURN_PHASE_BRAKE;
                    phase_started_ms = now;
                    settle_samples = 0u;
                    brake_motors();
                } else {
                    pwm = fine_pwm;
                    apply_turn((float)fine_direction, pwm);
                }
            } else {
                brake_motors();
                if ((uint32_t)(now - phase_started_ms) >=
                        TURN_BRAKE_MIN_MS &&
                    abs_float(imu.gyro_dps[2]) <= TURN_SETTLE_RATE_DPS) {
                    settle_samples++;
                } else {
                    settle_samples = 0u;
                }

                if (settle_samples >= TURN_SETTLE_SAMPLES) {
                    float error_magnitude = abs_float(error_deg);

                    settle_samples = 0u;
                    if (error_magnitude <= TURN_PRECISION_DEG) {
                        stop_motors();
#ifdef NUEDC_ENCODER_CALIBRATION
                        if (state == TURN_STATE_CW ||
                            state == TURN_STATE_SEQUENCE_CW) {
                            s_cw_left_result = s_left_software_total;
                            s_cw_right_result = s_right_software_total;
                        } else {
                            s_ccw_left_result = s_left_software_total;
                            s_ccw_right_result = s_right_software_total;
                        }
#endif
                        if (state == TURN_STATE_SEQUENCE_CW) {
                            state = TURN_STATE_SEQUENCE_PAUSE;
                            phase_started_ms = now;
                        } else {
                            state = TURN_STATE_DONE;
                        }
                    } else if (fine_attempts >= TURN_FINE_MAX_ATTEMPTS) {
                        state = TURN_STATE_FAULT;
                        fault_status = -6;
                        status = fault_status;
                    } else {
                        if (fine_attempts > 0u) {
                            float fine_progress =
                                (imu.yaw_deg - fine_start_yaw_deg) *
                                (float)fine_direction;

                            if (fine_progress <
                                TURN_FINE_PROGRESS_MIN_DEG) {
                                fine_pwm = clamp_fine_pwm(
                                    (int16_t)(fine_pwm +
                                              TURN_FINE_PWM_STEP));
                            } else if (error_magnitude >=
                                       fine_error_before_deg) {
                                fine_pwm = clamp_fine_pwm(
                                    (int16_t)(fine_pwm -
                                              TURN_FINE_PWM_STEP));
                            }
                        }
                        fine_direction = (error_deg >= 0.0f) ? 1 : -1;
                        fine_start_yaw_deg = imu.yaw_deg;
                        fine_error_before_deg = error_magnitude;
                        fine_attempts++;
                        phase = TURN_PHASE_FINE_PULSE;
                        phase_started_ms = now;
                        pwm = fine_pwm;
                        apply_turn((float)fine_direction, pwm);
                    }
                }
            }
        } else if (state == TURN_STATE_SEQUENCE_PAUSE) {
            error_deg = target_deg - imu.yaw_deg;
            stop_motors();
            if ((uint32_t)(now - phase_started_ms) >= SEQUENCE_PAUSE_MS) {
                state = TURN_STATE_SEQUENCE_CCW;
                turn_origin_deg = imu.yaw_deg;
                expected_direction = 1;
                target_deg += TURN_ANGLE_DEG;
                settle_samples = 0u;
                phase = TURN_PHASE_COARSE;
                fine_pwm = TURN_FINE_PWM_INITIAL;
                fine_attempts = 0u;
                phase_started_ms = now;
                segment_started_ms = now;
#ifdef NUEDC_ENCODER_CALIBRATION
                encoder_diagnostic_reset();
#endif
            }
        } else {
            if (state == TURN_STATE_DONE || state == TURN_STATE_FAULT) {
                error_deg = target_deg - imu.yaw_deg;
            }
            stop_motors();
        }

        if (state == TURN_STATE_FAULT) {
            stop_motors();
        }
        BSP_Motor_SafetyTick(now);
        publish_turn_snapshot(state, &imu, target_deg, error_deg, pwm, status,
                              phase, fine_attempts);
        System_Health_Beat(HEALTH_TASK_FAST_IO, now);
        System_Health_Beat(HEALTH_TASK_CONTROL, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

static bool oled_init_auto_address(void)
{
    static const uint8_t addresses[] = {
        SSD1306_I2C_ADDRESS_DEFAULT,
        SSD1306_I2C_ADDRESS_ALT,
    };

    for (uint8_t i = 0u; i < (sizeof(addresses) / sizeof(addresses[0])); ++i) {
        s_oled_address = addresses[i];
        if (SSD1306_MSPM0_Init(OLED_I2C_INST, s_oled_address)) {
            return true;
        }
    }
    return false;
}

static void oled_write(uint8_t row, const char *text)
{
    if (s_oled_ok &&
        !SSD1306_MSPM0_WriteStringPadded(row, 0u, text)) {
        s_oled_ok = false;
    }
}

static void format_tenths(char *buffer, size_t size, const char *label,
                          float value)
{
    int32_t tenths = to_tenths(value);
    int32_t magnitude = (tenths < 0) ? -tenths : tenths;

    (void)snprintf(buffer, size, "%s:%c%ld.%01ld", label,
                   (tenths < 0) ? '-' : '+',
                   (long)(magnitude / 10), (long)(magnitude % 10));
}

static void ui_task(void *argument)
{
    TickType_t last;

    (void)argument;
    vTaskDelay(pdMS_TO_TICKS(100u));
    s_oled_ok = oled_init_auto_address();
#ifdef NUEDC_ENCODER_CALIBRATION
    uart_printf("\r\nENCODER IMU 360 CALIBRATION\r\n");
    uart_printf("PA7=CW360 PB1=CCW360 PB21=CW360+CCW360\r\n");
#else
    uart_printf("\r\nIMU TURN COORDINATION TEST\r\n");
    uart_printf("PA7=CW90 PB1=CCW90 PB21=CW90+CCW90\r\n");
#endif
    uart_printf("$OLED,%s,0x%02X\r\n", s_oled_ok ? "OK" : "ERROR",
                s_oled_address);
    last = xTaskGetTickCount();

    for (;;) {
        TurnSnapshot snapshot;
        char line[SSD1306_TEXT_COLUMNS + 1u];
        uint32_t now = (uint32_t)xTaskGetTickCount();

        read_turn_snapshot(&snapshot);
#ifdef NUEDC_ENCODER_CALIBRATION
        oled_write(0u, "ENCODER YAW CAL");
#else
        oled_write(0u, "IMU TURN TEST");
#endif
        (void)snprintf(line, sizeof(line), "STATE:%s",
                       state_text(snapshot.state));
        oled_write(1u, line);
#ifdef NUEDC_ENCODER_CALIBRATION
        (void)snprintf(line, sizeof(line), "AB:L%X/%u R%X/%u",
                       snapshot.left_seen_mask, snapshot.left_raw,
                       snapshot.right_seen_mask, snapshot.right_raw);
        oled_write(2u, line);
#endif
        format_tenths(line, sizeof(line), "YAW", snapshot.yaw_deg);
        oled_write(3u, line);
        format_tenths(line, sizeof(line), "TGT", snapshot.target_deg);
        oled_write(4u, line);
        format_tenths(line, sizeof(line), "ERR", snapshot.error_deg);
        oled_write(5u, line);
#ifdef NUEDC_ENCODER_CALIBRATION
        (void)snprintf(line, sizeof(line), "L:%lld",
                       (long long)snapshot.left_total);
        oled_write(6u, line);
        (void)snprintf(line, sizeof(line), "R:%lld",
                       (long long)snapshot.right_total);
        oled_write(7u, line);
        uart_printf("$ECAL,%s,%s,%ld,%ld,%ld,%lld,%lld,%d,%u,%d,"
                    "%lld,%lld,%lld,%lld,%u,%u,%u,%u\r\n",
                    state_text(snapshot.state),
                    phase_text(snapshot.phase),
                    (long)to_millidegrees(snapshot.yaw_deg),
                    (long)to_millidegrees(snapshot.target_deg),
                    (long)to_millidegrees(snapshot.error_deg),
                    (long long)snapshot.left_total,
                    (long long)snapshot.right_total,
                    snapshot.pwm, (unsigned int)snapshot.fine_attempts,
                    snapshot.status,
                    (long long)snapshot.cw_left_result,
                    (long long)snapshot.cw_right_result,
                    (long long)snapshot.ccw_left_result,
                    (long long)snapshot.ccw_right_result,
                    snapshot.left_seen_mask, snapshot.left_raw,
                    snapshot.right_seen_mask, snapshot.right_raw);
#else
        (void)snprintf(line, sizeof(line), "%s N:%u P:%d",
                       phase_text(snapshot.phase),
                       (unsigned int)snapshot.fine_attempts, snapshot.pwm);
        oled_write(7u, line);
        uart_printf("$TURN,%s,%s,%ld,%ld,%ld,%d,%u,%d\r\n",
                    state_text(snapshot.state),
                    phase_text(snapshot.phase),
                    (long)to_millidegrees(snapshot.yaw_deg),
                    (long)to_millidegrees(snapshot.target_deg),
                    (long)to_millidegrees(snapshot.error_deg),
                    snapshot.pwm, (unsigned int)snapshot.fine_attempts,
                    snapshot.status);
#endif
        System_Health_Beat(HEALTH_TASK_TELEMETRY, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(UI_PERIOD_MS));
    }
}

static void health_task(void *argument)
{
    TickType_t last = xTaskGetTickCount();

    (void)argument;
    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();
        uint32_t stale_mask = 0u;

        if (System_Health_Check(now, &stale_mask) &&
            !System_Fault_IsActive()) {
            System_Watchdog_Feed();
        } else if (stale_mask != 0u) {
            System_Fault_Raise(SYSTEM_FAULT_TASK_STALE, stale_mask);
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(20u));
    }
}

int main(void)
{
    SYSCFG_DL_init();
    System_Watchdog_Init();
    BSP_Motor_Init();
#ifdef NUEDC_ENCODER_CALIBRATION
    encoder_gpio_diagnostic_init();
#endif
    IMU_Service_Init();
    System_Fault_Init();
    System_Health_Init(0u);
    publish_turn_snapshot(TURN_STATE_WAIT_IMU, 0, 0.0f, 0.0f, 0, 0,
                          TURN_PHASE_COARSE, 0u);

    if (xTaskCreateStatic(health_task, "health", HEALTH_STACK_WORDS, 0, 5u,
                          s_health_stack, &s_health_tcb) == 0 ||
        xTaskCreateStatic(IMU_Service_Task, "imu", IMU_STACK_WORDS, 0, 4u,
                          s_imu_stack, &s_imu_tcb) == 0 ||
        xTaskCreateStatic(control_task, "turn", CONTROL_STACK_WORDS, 0, 3u,
                          s_control_stack, &s_control_tcb) == 0 ||
        xTaskCreateStatic(ui_task, "oled", UI_STACK_WORDS, 0, 2u,
                          s_ui_stack, &s_ui_tcb) == 0) {
        taskDISABLE_INTERRUPTS();
        for (;;) {
        }
    }
    vTaskStartScheduler();
    stop_motors();
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    stop_motors();
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    stop_motors();
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationAssertHook(const char *file, int line)
{
    (void)file;
    (void)line;
    stop_motors();
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationGetIdleTaskMemory(StaticTask_t **task_buffer,
                                   StackType_t **stack_buffer,
                                   configSTACK_DEPTH_TYPE *stack_size)
{
    *task_buffer = &s_idle_tcb;
    *stack_buffer = s_idle_stack;
    *stack_size = configMINIMAL_STACK_SIZE;
}
