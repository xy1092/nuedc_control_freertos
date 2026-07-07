#include "ti_msp_dl_config.h"
#include "app/app_main.h"
#include "bsp/bsp_motor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

volatile uint32_t g_tick_ms = 0;

int main(void)
{
    SYSCFG_DL_init();
    App_Init();
    vTaskStartScheduler();

    BSP_Motor_SetDuty(0, 0);
    BSP_Motor_SetDuty(1, 0);
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    BSP_Motor_SetDuty(0, 0);
    BSP_Motor_SetDuty(1, 0);
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    BSP_Motor_SetDuty(0, 0);
    BSP_Motor_SetDuty(1, 0);
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}
