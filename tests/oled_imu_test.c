#include "ti_msp_dl_config.h"
#include "bsp_imu.h"
#include "dev_icm45688.h"
#include "drv_uart.h"
#include "imu_profile.h"
#include "pin_map.h"
#include "ssd1306_mspm0.h"
#include "watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define IMU_DATA_TIMEOUT_MS  50u
#define IMU_EXPECTED_WHOAMI  0xE9u
#define IMU_INIT_RETRIES     5u
#define IMU_TASK_STACK_WORDS  1024u
#define UI_TASK_STACK_WORDS   512u
#define UI_PERIOD_MS          100u

typedef enum {
    TEST_IMU_INITIALIZING = 0,
    TEST_IMU_CALIBRATING,
    TEST_IMU_READY,
    TEST_IMU_FAULT,
} TestImuState;

typedef struct {
    float yaw_deg;
    int status;
    TestImuState state;
    uint8_t whoami;
} TestImuSnapshot;

static icm45688_mspm0_t s_imu;
static TestImuSnapshot s_snapshot;
static bool s_oled_ok;
static uint8_t s_oled_address = SSD1306_I2C_ADDRESS_DEFAULT;
static const DrvUartPort s_uart = {
    UART_DEBUG_INST, UART_DEBUG_INST_INT_IRQN,
};

static StaticTask_t s_imu_tcb;
static StaticTask_t s_ui_tcb;
static StackType_t s_imu_stack[IMU_TASK_STACK_WORDS];
static StackType_t s_ui_stack[UI_TASK_STACK_WORDS];
static StaticTask_t s_idle_tcb;
static StackType_t s_idle_stack[configMINIMAL_STACK_SIZE];

static void uart_printf(const char *format, ...)
{
    char text[160];
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

static void delay_with_watchdog(uint32_t delay_ms)
{
    while (delay_ms > 0u) {
        uint32_t step_ms = (delay_ms > 10u) ? 10u : delay_ms;

        vTaskDelay(pdMS_TO_TICKS(step_ms));
        System_Watchdog_Feed();
        delay_ms -= step_ms;
    }
}

static void oled_write(uint8_t row, const char *text)
{
    if (s_oled_ok &&
        !SSD1306_MSPM0_WriteStringPadded(row, 0u, text)) {
        s_oled_ok = false;
        uart_printf("$OLED,WRITE_ERROR\r\n");
    }
}

static void oled_show_status(const char *state, const char *detail)
{
    oled_write(0u, "ICM45688 YAW");
    oled_write(2u, state);
    oled_write(4u, detail);
}

static bool oled_init_auto_address(void)
{
    static const uint8_t addresses[] = {
        SSD1306_I2C_ADDRESS_DEFAULT,
        SSD1306_I2C_ADDRESS_ALT,
    };

    for (uint8_t i = 0u; i < (sizeof(addresses) / sizeof(addresses[0])); ++i) {
        s_oled_address = addresses[i];
        if (SSD1306_MSPM0_Init(I2C_OLED_INST, s_oled_address)) {
            return true;
        }
    }
    return false;
}

static void publish_snapshot(TestImuState state, int status, uint8_t whoami)
{
    taskENTER_CRITICAL();
    s_snapshot.yaw_deg = s_imu.yaw_deg;
    s_snapshot.status = status;
    s_snapshot.state = state;
    s_snapshot.whoami = whoami;
    taskEXIT_CRITICAL();
}

static void read_snapshot(TestImuSnapshot *snapshot)
{
    taskENTER_CRITICAL();
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL();
}

static float sample_stddev(float sum, float sum_sq, uint32_t count)
{
    float mean;
    float variance;

    if (count == 0u) {
        return 0.0f;
    }
    mean = sum / (float)count;
    variance = (sum_sq / (float)count) - (mean * mean);
    if (variance < 0.0f) {
        variance = 0.0f;
    }
    return sqrtf(variance);
}

static void configure_imu_handle(void)
{
    const ImuCalibratedProfile_t *profile = &g_imu_calibrated_profile;

    ICM45688_MSPM0_InitHandleSPI(&s_imu, BSP_IMU_GetBus());
    ICM45688_MSPM0_SetFilter(&s_imu, profile->sample_period_s,
                             profile->filter_alpha);
    ICM45688_MSPM0_SetSampleLowPass(&s_imu, profile->accel_lpf_hz,
                                    profile->gyro_lpf_hz);
    ICM45688_MSPM0_SetStationaryDetection(
        &s_imu, profile->stationary_gyro_dps, profile->stationary_accel_g,
        profile->stationary_hold_samples);
    ICM45688_MSPM0_SetAttitudeMode(
        &s_imu, ICM45688_MSPM0_ATTITUDE_MAHONY_6AXIS);
    ICM45688_MSPM0_SetAccelTrustWindow(&s_imu,
                                       profile->accel_trust_window_g);
    ICM45688_MSPM0_SetAccelRejection(&s_imu,
                                     profile->accel_rejection_deg);
    ICM45688_MSPM0_SetMahonyGains(&s_imu, profile->mahony_kp,
                                  profile->mahony_ki);
}

static int wait_and_update_imu(uint32_t timeout_ms)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    for (;;) {
        if (BSP_IMU_DataReady()) {
            uint8_t data_ready = 0u;
            int status = ICM45688_MSPM0_GetDataReady(&s_imu, &data_ready);

            if (status != INV_IMU_OK) {
                return status;
            }
            if (data_ready) {
                return ICM45688_MSPM0_Update(&s_imu);
            }
        }
        if ((xTaskGetTickCount() - start) >= timeout_ticks) {
            return INV_IMU_ERROR_TIMEOUT;
        }
        System_Watchdog_Feed();
        vTaskDelay(pdMS_TO_TICKS(1u));
    }
}

static int calibrate_gyro_bias(void)
{
    const ImuCalibratedProfile_t *profile = &g_imu_calibrated_profile;

    for (;;) {
        float gx_sum = 0.0f;
        float gy_sum = 0.0f;
        float gz_sum = 0.0f;
        float gx_sq_sum = 0.0f;
        float gy_sq_sum = 0.0f;
        float gz_sq_sum = 0.0f;
        float gx_stddev;
        float gy_stddev;
        float gz_stddev;

        ICM45688_MSPM0_SetGyroScale(&s_imu, 1.0f, 1.0f, 1.0f);
        ICM45688_MSPM0_SetGyroBias(&s_imu, 0.0f, 0.0f, 0.0f);
        ICM45688_MSPM0_SetGyroBiasAutoUpdate(&s_imu, 0u, 0.0f);
        ICM45688_MSPM0_ResetSampleLowPass(&s_imu);
        ICM45688_MSPM0_ResetAttitude(&s_imu);

        for (uint32_t i = 0u;
             i < (uint32_t)(profile->gyro_cal_warmup_samples +
                            profile->gyro_cal_samples);
             ++i) {
            int status = wait_and_update_imu(IMU_DATA_TIMEOUT_MS);

            if (status != INV_IMU_OK) {
                return status;
            }
            if (i < profile->gyro_cal_warmup_samples) {
                continue;
            }
            gx_sum += s_imu.gx_dps;
            gy_sum += s_imu.gy_dps;
            gz_sum += s_imu.gz_dps;
            gx_sq_sum += s_imu.gx_dps * s_imu.gx_dps;
            gy_sq_sum += s_imu.gy_dps * s_imu.gy_dps;
            gz_sq_sum += s_imu.gz_dps * s_imu.gz_dps;
        }

        gx_stddev = sample_stddev(gx_sum, gx_sq_sum,
                                  profile->gyro_cal_samples);
        gy_stddev = sample_stddev(gy_sum, gy_sq_sum,
                                  profile->gyro_cal_samples);
        gz_stddev = sample_stddev(gz_sum, gz_sq_sum,
                                  profile->gyro_cal_samples);
        if (gx_stddev > profile->gyro_cal_stddev_limit_dps ||
            gy_stddev > profile->gyro_cal_stddev_limit_dps ||
            gz_stddev > profile->gyro_cal_stddev_limit_dps) {
            delay_with_watchdog(500u);
            continue;
        }

        ICM45688_MSPM0_SetGyroBias(
            &s_imu,
            gx_sum / (float)profile->gyro_cal_samples,
            gy_sum / (float)profile->gyro_cal_samples,
            gz_sum / (float)profile->gyro_cal_samples);
        ICM45688_MSPM0_SetGyroScale(&s_imu, 1.0f, 1.0f,
                                    profile->gyro_z_scale);
        ICM45688_MSPM0_SetGyroBiasAutoUpdate(
            &s_imu, 1u, profile->gyro_bias_learning_rate);
        ICM45688_MSPM0_ResetSampleLowPass(&s_imu);
        ICM45688_MSPM0_ResetAttitude(&s_imu);
        return INV_IMU_OK;
    }
}

static int32_t yaw_to_millidegrees(float yaw_deg)
{
    float scaled = yaw_deg * 1000.0f;

    return (int32_t)((scaled >= 0.0f) ? scaled + 0.5f : scaled - 0.5f);
}

static void format_yaw(char line[SSD1306_TEXT_COLUMNS + 1u], float yaw_deg)
{
    int32_t milli = yaw_to_millidegrees(yaw_deg);
    int32_t magnitude = (milli < 0) ? -milli : milli;

    (void)snprintf(line, SSD1306_TEXT_COLUMNS + 1u,
                   "YAW:%c%ld.%03ld deg", (milli < 0) ? '-' : '+',
                   (long)(magnitude / 1000), (long)(magnitude % 1000));
}

static void imu_fault_loop(int status, uint8_t whoami)
{
    publish_snapshot(TEST_IMU_FAULT, status, whoami);
    for (;;) {
        System_Watchdog_Feed();
        DL_GPIO_togglePins(STATUS_LED_PORT, STATUS_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(250u));
    }
}

static void imu_task(void *argument)
{
    uint8_t whoami = 0u;
    int status = INV_IMU_ERROR;

    (void)argument;
    publish_snapshot(TEST_IMU_INITIALIZING, status, whoami);

    for (uint8_t attempt = 1u; attempt <= IMU_INIT_RETRIES; ++attempt) {
        configure_imu_handle();
        status = ICM45688_MSPM0_ReadWhoAmI(&s_imu, &whoami);
        if (status == INV_IMU_OK && whoami == IMU_EXPECTED_WHOAMI) {
            status = ICM45688_MSPM0_Begin(&s_imu);
        }
        if (status == INV_IMU_OK) {
            break;
        }
        publish_snapshot(TEST_IMU_INITIALIZING, status, whoami);
        delay_with_watchdog(200u);
    }
    if (status != INV_IMU_OK) {
        imu_fault_loop(status, whoami);
    }

    publish_snapshot(TEST_IMU_CALIBRATING, status, whoami);
    status = calibrate_gyro_bias();
    if (status != INV_IMU_OK) {
        imu_fault_loop(status, whoami);
    }

    publish_snapshot(TEST_IMU_READY, status, whoami);

    for (;;) {
        status = wait_and_update_imu(IMU_DATA_TIMEOUT_MS);
        if (status != INV_IMU_OK) {
            imu_fault_loop(status, whoami);
        }
        publish_snapshot(TEST_IMU_READY, status, whoami);
    }
}

static void ui_task(void *argument)
{
    TickType_t last;
    TestImuState previous_state = (TestImuState)0xFFu;

    (void)argument;
    vTaskDelay(pdMS_TO_TICKS(100u));
    s_oled_ok = oled_init_auto_address();
    uart_printf("\r\nOLED + ICM45688 YAW TEST\r\n");
    uart_printf("$OLED,%s,0x%02X\r\n", s_oled_ok ? "OK" : "ERROR",
                s_oled_address);
    last = xTaskGetTickCount();

    for (;;) {
        TestImuSnapshot snapshot;

        read_snapshot(&snapshot);
        if (snapshot.state != previous_state) {
            previous_state = snapshot.state;
            switch (snapshot.state) {
            case TEST_IMU_CALIBRATING:
                oled_show_status("CALIBRATING", "KEEP STILL");
                uart_printf("$IMU,CALIBRATING\r\n");
                break;
            case TEST_IMU_READY:
                oled_write(0u, "ICM45688");
                oled_write(2u, "YAW ANGLE");
                oled_write(4u, "YAW ZEROED");
                uart_printf("$IMU,READY,0x%02X\r\n", snapshot.whoami);
                break;
            case TEST_IMU_FAULT:
                oled_show_status("ERROR", "CHECK IMU");
                uart_printf("$IMU,ERROR,%d,0x%02X\r\n",
                            snapshot.status, snapshot.whoami);
                break;
            case TEST_IMU_INITIALIZING:
            default:
                oled_show_status("INITIALIZING", "WAIT IMU");
                uart_printf("$IMU,INITIALIZING\r\n");
                break;
            }
        }

        if (snapshot.state == TEST_IMU_READY) {
            char line[SSD1306_TEXT_COLUMNS + 1u];

            format_yaw(line, snapshot.yaw_deg);
            oled_write(4u, line);
            uart_printf("$YAW,%ld\r\n",
                        (long)yaw_to_millidegrees(snapshot.yaw_deg));
        }
        vTaskDelayUntil(&last, pdMS_TO_TICKS(UI_PERIOD_MS));
    }
}

int main(void)
{
    SYSCFG_DL_init();
    System_Watchdog_Init();
    System_Watchdog_Feed();
    publish_snapshot(TEST_IMU_INITIALIZING, INV_IMU_ERROR, 0u);
    if (xTaskCreateStatic(imu_task, "imu", IMU_TASK_STACK_WORDS,
                          0, 4u, s_imu_stack, &s_imu_tcb) == 0) {
        for (;;) {
        }
    }
    if (xTaskCreateStatic(ui_task, "oled", UI_TASK_STACK_WORDS,
                          0, 2u, s_ui_stack, &s_ui_tcb) == 0) {
        for (;;) {
        }
    }
    vTaskStartScheduler();
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationAssertHook(const char *file, int line)
{
    (void)file;
    (void)line;
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
