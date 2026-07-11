#ifndef SYSTEM_FAULT_MANAGER_H
#define SYSTEM_FAULT_MANAGER_H

#include <stdint.h>

typedef enum {
    SYSTEM_FAULT_NONE = 0,
    SYSTEM_FAULT_TASK_STALE = 1,
    SYSTEM_FAULT_IMU = 2,
    SYSTEM_FAULT_STACK = 3,
    SYSTEM_FAULT_HEAP = 4,
} SystemFaultCode_t;

void System_Fault_Init(void);
void System_Fault_Raise(SystemFaultCode_t code, uint32_t detail);
SystemFaultCode_t System_Fault_Get(uint32_t *detail);
uint8_t System_Fault_IsActive(void);

#endif
