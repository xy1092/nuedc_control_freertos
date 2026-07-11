#include "bsp_uart.h"
#include "pin_map.h"
#include "drv_uart.h"
#include "rtt_log.h"
#include <stdarg.h>
#include <stdio.h>

static uart_rx_cb_t s_cb_dbg = 0;
static const DrvUartPort s_debug_uart = { UART_DEBUG_INST, UART_DEBUG_INST_INT_IRQN };

void BSP_Uart_Init(void)
{
    Drv_Uart_EnableRxInterrupt(&s_debug_uart);
}

void BSP_Uart_SetRxCallback(uint8_t is_openmv, uart_rx_cb_t cb)
{
    (void)is_openmv;
    s_cb_dbg = cb;
}

void BSP_Uart_Send(uint8_t is_openmv, const uint8_t *data, uint16_t len)
{
    (void)is_openmv;
    RTT_Log_Write((const char *)data, len);
    Drv_Uart_WriteBlocking(&s_debug_uart, data, len);
}

void BSP_Uart_Printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0) BSP_Uart_Send(0, (uint8_t *)buf, (uint16_t)n);
}

/* ===== ISR（函数名以 SysConfig 为准）===== */
void UART_DEBUG_INST_IRQHandler(void)
{
    uint8_t b;

    if (Drv_Uart_ReadIrqByte(&s_debug_uart, &b)) {
        if (s_cb_dbg) s_cb_dbg(b);
    }
}
