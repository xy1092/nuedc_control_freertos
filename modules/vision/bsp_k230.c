#include "bsp_k230.h"
#include "pin_map.h"
#include "drv_gpio.h"
#include "drv_uart.h"

static k230_rx_cb_t s_k230_cb = 0;

#ifdef K230_UART_INST
static const DrvUartPort s_k230_uart = { K230_UART_INST, K230_UART_IRQN };
#endif
#ifdef K230_RESET_PIN
static const DrvGpioPin s_k230_reset = { K230_RESET_PORT, K230_RESET_PIN, 0u };
#endif
#ifdef K230_READY_PIN
static const DrvGpioPin s_k230_ready = { K230_READY_PORT, K230_READY_PIN, 0u };
#endif

void BSP_K230_Init(void)
{
#ifdef K230_UART_INST
    Drv_Uart_EnableRxInterrupt(&s_k230_uart);
#endif
#ifdef K230_RESET_PIN
    Drv_Gpio_WriteRaw(&s_k230_reset, 1u);
#endif
}

void BSP_K230_SetRxCallback(k230_rx_cb_t cb)
{
    s_k230_cb = cb;
}

void BSP_K230_Send(const uint8_t *data, uint16_t len)
{
#ifdef K230_UART_INST
    Drv_Uart_WriteBlocking(&s_k230_uart, data, len);
#else
    (void)data;
    (void)len;
#endif
}

void BSP_K230_Reset(uint8_t hold_reset)
{
#ifdef K230_RESET_PIN
    Drv_Gpio_WriteRaw(&s_k230_reset, hold_reset ? 0u : 1u);
#else
    (void)hold_reset;
#endif
}

uint8_t BSP_K230_IsReady(void)
{
#ifdef K230_READY_PIN
    return Drv_Gpio_ReadRaw(&s_k230_ready);
#else
    return 0u;
#endif
}

#ifdef K230_UART_INST
void UART_K230_INST_IRQHandler(void)
{
    uint8_t b;

    if (Drv_Uart_ReadIrqByte(&s_k230_uart, &b)) {
        if (s_k230_cb) s_k230_cb(b);
    }
}
#endif
