#include "ti_msp_dl_config.h"
#include "bsp_adc.h"
#include "bsp_encoder.h"
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

#define STRAIGHT_DISTANCE_CM       100u
#define BASE_PWM                   300
#define START_PWM                  300
#define LEFT_PWM_OFFSET            (-2)
#define RIGHT_PWM_OFFSET           2
#define RIGHT_PULSES_PER_CM        73.14f
#define ENCODER_TARGET_PULSES      7314u
#define DECEL_WINDOW_PULSES        900u
#define MIN_CLOSED_LOOP_PWM        220
#define RAMP_TIME_MS               500u
#define RUN_TIMEOUT_MS             15000u
#define BRAKE_HOLD_MS              300u
#define FINISH_ARM_MS              500u
#define LINE_STABLE_SAMPLES        5u
#define LINE_CENTER_INDEX          3u
#define FAST_PERIOD_MS             1u
#define CONTROL_PERIOD_MS          10u
#define UI_PERIOD_MS               100u
#define DEBOUNCE_SAMPLES           3u
#define IMU_STACK_WORDS            768u
#define FAST_STACK_WORDS           256u
#define CONTROL_STACK_WORDS        384u
#define UI_STACK_WORDS             512u
#define HEALTH_STACK_WORDS         256u

typedef enum {
    STRAIGHT_WAIT_IMU = 0,
    STRAIGHT_READY,
    STRAIGHT_RUNNING,
    STRAIGHT_DONE,
    STRAIGHT_FAULT,
} StraightState;

typedef struct {
    StraightState state;
    int64_t left_total;
    int64_t right_total;
    float yaw_deg;
    float yaw_reference_deg;
    float yaw_error_deg;
    int16_t left_pwm;
    int16_t right_pwm;
    uint32_t elapsed_ms;
    int status;
    uint8_t center_level;
    uint8_t start_black_level;
    uint8_t left_start_line;
} StraightSnapshot;

volatile uint32_t g_tick_ms;

static StraightSnapshot s_snapshot;
static bool s_oled_ok;
static uint8_t s_oled_address = SSD1306_I2C_ADDRESS_DEFAULT;
static const DrvGpioPin s_start_stop_button = {
    START_STOP_BUTTON_PORT, START_STOP_BUTTON_PIN, 1u,
};
static const DrvUartPort s_uart = {
    UART_DEBUG_INST, UART_DEBUG_INST_INT_IRQN,
};

static StaticTask_t s_imu_tcb;
static StaticTask_t s_fast_tcb;
static StaticTask_t s_control_tcb;
static StaticTask_t s_ui_tcb;
static StaticTask_t s_health_tcb;
static StaticTask_t s_idle_tcb;
static StackType_t s_imu_stack[IMU_STACK_WORDS];
static StackType_t s_fast_stack[FAST_STACK_WORDS];
static StackType_t s_control_stack[CONTROL_STACK_WORDS];
static StackType_t s_ui_stack[UI_STACK_WORDS];
static StackType_t s_health_stack[HEALTH_STACK_WORDS];
static StackType_t s_idle_stack[configMINIMAL_STACK_SIZE];

static int32_t to_millidegrees(float value)
{
    float scaled = value * 1000.0f;

    return (int32_t)((scaled >= 0.0f) ? scaled + 0.5f : scaled - 0.5f);
}

static int64_t abs_i64(int64_t value)
{
    return (value < 0) ? -value : value;
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

static const char *state_text(StraightState state)
{
    switch (state) {
    case STRAIGHT_READY: return "READY";
    case STRAIGHT_RUNNING: return "RUNNING";
    case STRAIGHT_DONE: return "DONE";
    case STRAIGHT_FAULT: return "FAULT";
    case STRAIGHT_WAIT_IMU:
    default: return "WAIT IMU";
    }
}

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

static void publish_snapshot(StraightState state, const ImuSnapshot_t *imu,
                             float yaw_reference, float yaw_error,
                             int16_t left_pwm, int16_t right_pwm,
                             uint32_t elapsed_ms, int status,
                             uint8_t center_level,
                             uint8_t start_black_level,
                             uint8_t left_start_line)
{
    taskENTER_CRITICAL();
    s_snapshot.state = state;
    s_snapshot.left_total = BSP_Encoder_GetTotal(0u);
    s_snapshot.right_total = BSP_Encoder_GetTotal(1u);
    s_snapshot.yaw_deg = (imu != 0) ? imu->yaw_deg : 0.0f;
    s_snapshot.yaw_reference_deg = yaw_reference;
    s_snapshot.yaw_error_deg = yaw_error;
    s_snapshot.left_pwm = left_pwm;
    s_snapshot.right_pwm = right_pwm;
    s_snapshot.elapsed_ms = elapsed_ms;
    s_snapshot.status = status;
    s_snapshot.center_level = center_level;
    s_snapshot.start_black_level = start_black_level;
    s_snapshot.left_start_line = left_start_line;
    taskEXIT_CRITICAL();
}

static void read_snapshot(StraightSnapshot *snapshot)
{
    taskENTER_CRITICAL();
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL();
}

static void fast_task(void *argument)
{
    TickType_t last = xTaskGetTickCount();

    (void)argument;
    for (;;) {
        uint32_t now = (uint32_t)xTaskGetTickCount();

        g_tick_ms = now;
        BSP_Encoder_Poll();
        BSP_Motor_SafetyTick(now);
        System_Health_Beat(HEALTH_TASK_FAST_IO, now);
        vTaskDelayUntil(&last, pdMS_TO_TICKS(FAST_PERIOD_MS));
    }
}

static bool read_button_press_event(void)
{
    static uint8_t candidate;
    static uint8_t stable;
    static uint8_t stable_samples = DEBOUNCE_SAMPLES;
    uint8_t sample = Drv_Gpio_ReadActive(&s_start_stop_button);

    if (sample != candidate) {
        candidate = sample;
        stable_samples = 1u;
    } else if (stable_samples < DEBOUNCE_SAMPLES) {
        stable_samples++;
    }
    if (stable_samples >= DEBOUNCE_SAMPLES && stable != candidate) {
        stable = candidate;
        return stable != 0u;
    }
    return false;
}

static void control_task(void *argument)
{
    TickType_t last = xTaskGetTickCount();
    StraightState state = STRAIGHT_READY;
    float yaw_reference = 0.0f;
    uint32_t run_started_ms = 0u;
    uint32_t brake_started_ms = 0u;
    int fault_status = 0;
    uint8_t start_black_level = 0u;
    uint8_t left_start_line = 0u;
    uint8_t line_stable_samples = 0u;

    (void)argument;
    for (;;) {
        ImuSnapshot_t imu;
        uint32_t now = (uint32_t)xTaskGetTickCount();
        bool button_pressed = read_button_press_event();
        uint16_t line_raw[BSP_ADC_CCD_COUNT];
        uint8_t center_level;
        float yaw_error = 0.0f;
        int16_t left_pwm = 0;
        int16_t right_pwm = 0;
        uint32_t elapsed_ms = 0u;

        g_tick_ms = now;
        IMU_GetSnapshot(&imu);
        BSP_Adc_GetCCD(line_raw);
        center_level = (line_raw[LINE_CENTER_INDEX] != 0u) ? 1u : 0u;
        if (System_Fault_IsActive()) {
            state = STRAIGHT_FAULT;
            if (fault_status == 0) {
                fault_status = -5;
            }
        }

        if (button_pressed &&
            (state == STRAIGHT_READY || state == STRAIGHT_DONE)) {
            BSP_Encoder_Reset(0u);
            BSP_Encoder_Reset(1u);
            yaw_reference = imu.yaw_deg;
            run_started_ms = now;
            fault_status = 0;
            start_black_level = center_level;
            left_start_line = 0u;
            line_stable_samples = 0u;
            state = STRAIGHT_RUNNING;
        } else if (button_pressed && state == STRAIGHT_RUNNING) {
            brake_motors();
            brake_started_ms = now;
            state = STRAIGHT_DONE;
        }

        if (state == STRAIGHT_RUNNING) {
            uint32_t run_elapsed = (uint32_t)(now - run_started_ms);

            if (!left_start_line) {
                if (center_level != start_black_level) {
                    if (++line_stable_samples >= LINE_STABLE_SAMPLES) {
                        left_start_line = 1u;
                        line_stable_samples = 0u;
                    }
                } else {
                    line_stable_samples = 0u;
                }
            } else if (run_elapsed >= FINISH_ARM_MS &&
                       (fault_status & 2) == 0) {
                uint8_t line_detected = 0u;
                uint8_t i;

                for (i = 0u; i < BSP_ADC_CCD_COUNT; ++i) {
                    uint8_t level = (line_raw[i] != 0u) ? 1u : 0u;

                    if (level == start_black_level) {
                        line_detected = 1u;
                        break;
                    }
                }
                if (line_detected) {
                    if (++line_stable_samples >= LINE_STABLE_SAMPLES) {
                        /* B line is validation; the encoder closes distance. */
                        fault_status |= 2;
                        line_stable_samples = 0u;
                    }
                } else {
                    line_stable_samples = 0u;
                }
            }
        }

        if (state == STRAIGHT_RUNNING) {
            uint32_t ramp_elapsed = (uint32_t)(now - run_started_ms);
            uint32_t right_count = (uint32_t)abs_i64(
                BSP_Encoder_GetTotal(1u));
            float base;
            float trim_scale;

            elapsed_ms = ramp_elapsed;
            yaw_error = yaw_reference - imu.yaw_deg;
            if (ramp_elapsed > RUN_TIMEOUT_MS) {
                state = STRAIGHT_FAULT;
                fault_status = -2;
            } else if (right_count >= ENCODER_TARGET_PULSES) {
                brake_motors();
                brake_started_ms = now;
                fault_status |= 1;
                state = STRAIGHT_DONE;
            } else {
                uint32_t remaining = ENCODER_TARGET_PULSES - right_count;

                if (remaining < DECEL_WINDOW_PULSES) {
                    base = (float)MIN_CLOSED_LOOP_PWM +
                           (float)(BASE_PWM - MIN_CLOSED_LOOP_PWM) *
                               (float)remaining /
                               (float)DECEL_WINDOW_PULSES;
                    trim_scale = 1.0f;
                } else if (ramp_elapsed < RAMP_TIME_MS) {
                    base = (float)START_PWM +
                           (float)(BASE_PWM - START_PWM) *
                               (float)ramp_elapsed / (float)RAMP_TIME_MS;
                    trim_scale = (float)ramp_elapsed /
                                 (float)RAMP_TIME_MS;
                } else {
                    base = (float)BASE_PWM;
                    trim_scale = 1.0f;
                }
                left_pwm = (int16_t)(base +
                    (float)LEFT_PWM_OFFSET * trim_scale);
                right_pwm = (int16_t)(base +
                    (float)RIGHT_PWM_OFFSET * trim_scale);
                BSP_Motor_SetSignedDuty(0u, left_pwm);
                BSP_Motor_SetSignedDuty(1u, right_pwm);
            }
        } else if (state == STRAIGHT_DONE) {
            elapsed_ms = (uint32_t)(brake_started_ms - run_started_ms);
            yaw_error = yaw_reference - imu.yaw_deg;
            if ((uint32_t)(now - brake_started_ms) < BRAKE_HOLD_MS) {
                brake_motors();
            } else {
                stop_motors();
            }
        } else {
            stop_motors();
        }

        if (state == STRAIGHT_FAULT) {
            stop_motors();
        }
        publish_snapshot(state, &imu, yaw_reference, yaw_error,
                         left_pwm, right_pwm, elapsed_ms, fault_status,
                         center_level, start_black_level, left_start_line);
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

static void ui_task(void *argument)
{
    TickType_t last;

    (void)argument;
    vTaskDelay(pdMS_TO_TICKS(100u));
    s_oled_ok = oled_init_auto_address();
    uart_printf("\r\n100 CM STRAIGHT ENCODER CALIBRATION\r\n");
    uart_printf("PB21=START/STOP BASE_PWM=300\r\n");
    uart_printf("$OLED,%s,0x%02X\r\n", s_oled_ok ? "OK" : "ERROR",
                s_oled_address);
    last = xTaskGetTickCount();

    for (;;) {
        StraightSnapshot snapshot;
        char line[SSD1306_TEXT_COLUMNS + 1u];
        uint32_t now = (uint32_t)xTaskGetTickCount();

        read_snapshot(&snapshot);
        oled_write(0u, "RIGHT ENC CLOSED");
        (void)snprintf(line, sizeof(line), "STATE:%s",
                       state_text(snapshot.state));
        oled_write(1u, line);
        (void)snprintf(line, sizeof(line), "G4:%u/%u %s",
                       snapshot.center_level, snapshot.start_black_level,
                       snapshot.left_start_line ? "SEEK B" : "LEAVE A");
        oled_write(2u, line);
        oled_write(3u, "L:OPEN LOOP FOLLOW");
        (void)snprintf(line, sizeof(line), "PULSE:%lld",
                       (long long)abs_i64(snapshot.right_total));
        oled_write(4u, line);
        {
            uint64_t right_count = (uint64_t)abs_i64(snapshot.right_total);
            uint32_t distance_tenths = (uint32_t)(
                (float)right_count * 10.0f / RIGHT_PULSES_PER_CM);

            (void)snprintf(line, sizeof(line), "DIST:%lu.%luCM",
                           (unsigned long)(distance_tenths / 10u),
                           (unsigned long)(distance_tenths % 10u));
            oled_write(5u, line);
            (void)snprintf(line, sizeof(line), "TARGET:%uP",
                           ENCODER_TARGET_PULSES);
        }
        oled_write(6u, line);
        (void)snprintf(line, sizeof(line), "T:%lu.%01lu S:%d",
                       (unsigned long)(snapshot.elapsed_ms / 1000u),
                       (unsigned long)((snapshot.elapsed_ms % 1000u) / 100u),
                       snapshot.status);
        oled_write(7u, line);
        uart_printf("$SCAL,%s,%lu,%lld,%lld,%ld,%ld,%d,%d,%d,%u,%u,%u\r\n",
                    state_text(snapshot.state),
                    (unsigned long)snapshot.elapsed_ms,
                    (long long)snapshot.left_total,
                    (long long)snapshot.right_total,
                    (long)to_millidegrees(snapshot.yaw_deg),
                    (long)to_millidegrees(snapshot.yaw_error_deg),
                    snapshot.left_pwm, snapshot.right_pwm, snapshot.status,
                    snapshot.center_level, snapshot.start_black_level,
                    snapshot.left_start_line);
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
    ImuSnapshot_t initial_imu = {0};

    SYSCFG_DL_init();
    System_Watchdog_Init();
    BSP_Motor_Init();
    BSP_Encoder_Init();
    BSP_Adc_Init();
    IMU_Service_Init();
    System_Fault_Init();
    System_Health_Init(0u);
    publish_snapshot(STRAIGHT_WAIT_IMU, &initial_imu, 0.0f, 0.0f,
                     0, 0, 0u, 0, 0u, 0u, 0u);

    if (xTaskCreateStatic(health_task, "health", HEALTH_STACK_WORDS, 0, 5u,
                          s_health_stack, &s_health_tcb) == 0 ||
        xTaskCreateStatic(IMU_Service_Task, "imu", IMU_STACK_WORDS, 0, 4u,
                          s_imu_stack, &s_imu_tcb) == 0 ||
        xTaskCreateStatic(fast_task, "enc", FAST_STACK_WORDS, 0, 4u,
                          s_fast_stack, &s_fast_tcb) == 0 ||
        xTaskCreateStatic(control_task, "straight", CONTROL_STACK_WORDS, 0, 3u,
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
