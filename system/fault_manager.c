#include "fault_manager.h"
#include "bsp_motor.h"

static volatile SystemFaultCode_t s_code;
static volatile uint32_t s_detail;

void System_Fault_Init(void)
{
    s_code = SYSTEM_FAULT_NONE;
    s_detail = 0u;
}

void System_Fault_Raise(SystemFaultCode_t code, uint32_t detail)
{
    BSP_Motor_SetDuty(0u, 0);
    BSP_Motor_SetDuty(1u, 0);
    if (s_code == SYSTEM_FAULT_NONE) {
        s_detail = detail;
        s_code = code;
    }
}

SystemFaultCode_t System_Fault_Get(uint32_t *detail)
{
    if (detail != 0) {
        *detail = s_detail;
    }
    return s_code;
}

uint8_t System_Fault_IsActive(void)
{
    return (s_code != SYSTEM_FAULT_NONE) ? 1u : 0u;
}
