#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

extern uint32_t SystemCoreClock;
extern void vApplicationAssertHook(const char *file, int line);

#define configENABLE_MPU                                0
#define configCPU_CLOCK_HZ                              ((unsigned long)32000000)
#define configTICK_RATE_HZ                              1000
#define configUSE_PREEMPTION                            1
#define configUSE_TIME_SLICING                          1
#define configUSE_TICKLESS_IDLE                         0
#define configMAX_PRIORITIES                            6
#define configMINIMAL_STACK_SIZE                        ((unsigned short)128)
#define configMAX_TASK_NAME_LEN                         16
#define configTICK_TYPE_WIDTH_IN_BITS                   TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                         1
#define configSUPPORT_DYNAMIC_ALLOCATION                1
#define configSUPPORT_STATIC_ALLOCATION                 1
#define configTOTAL_HEAP_SIZE                           (12 * 1024)
#define configAPPLICATION_ALLOCATED_HEAP                 0

#define configUSE_IDLE_HOOK                             0
#define configUSE_TICK_HOOK                             0
#define configCHECK_FOR_STACK_OVERFLOW                  2
#define configUSE_MALLOC_FAILED_HOOK                    1
#define configUSE_DAEMON_TASK_STARTUP_HOOK              0

#define configUSE_MUTEXES                               0
#define configUSE_RECURSIVE_MUTEXES                     0
#define configUSE_COUNTING_SEMAPHORES                   0
#define configUSE_QUEUE_SETS                            0
#define configQUEUE_REGISTRY_SIZE                       0

#define configUSE_TIMERS                                0
#define configUSE_EVENT_GROUPS                          0
#define configUSE_STREAM_BUFFERS                        0
#define configUSE_NEWLIB_REENTRANT                      0

#define configENABLE_BACKWARD_COMPATIBILITY             0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS         0
#define configCHECK_HANDLER_INSTALLATION                1

#define INCLUDE_vTaskDelay                              1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_xTaskGetTickCount                       1
#define INCLUDE_xTaskGetSchedulerState                  1
#define INCLUDE_vTaskDelete                             0
#define INCLUDE_vTaskSuspend                            0
#define INCLUDE_xTaskResumeFromISR                      0
#define INCLUDE_uxTaskPriorityGet                       0
#define INCLUDE_vTaskPrioritySet                        0
#define INCLUDE_eTaskGetState                           0

#define configASSERT(x)                                 \
    do {                                                \
        if ((x) == 0) {                                 \
            vApplicationAssertHook(__FILE__, __LINE__); \
        }                                               \
    } while (0)

#endif /* FREERTOS_CONFIG_H */
