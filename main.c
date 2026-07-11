#include "ti_msp_dl_config.h"
#include "app/app_main.h"
#include "bsp_motor.h"
#include "fault_manager.h"
#include "watchdog.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

volatile uint32_t g_tick_ms = 0;

static StaticTask_t s_idle_tcb;
static StackType_t s_idle_stack[configMINIMAL_STACK_SIZE];

int main(void)
{
    SYSCFG_DL_init();
    System_Watchdog_Init();
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
    System_Fault_Raise(SYSTEM_FAULT_HEAP, 0u);
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    System_Fault_Raise(SYSTEM_FAULT_STACK, 0u);
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationAssertHook(const char *file, int line)
{
    (void)file;
    System_Fault_Raise(SYSTEM_FAULT_STACK, (uint32_t)line);
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
