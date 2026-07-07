#include "drv_uart.h"

void Drv_Uart_EnableRxInterrupt(const DrvUartPort *uart)
{
    if ((uart == 0) || (uart->inst == 0)) {
        return;
    }

    DL_UART_Main_enableInterrupt(uart->inst, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_EnableIRQ(uart->irqn);
}

void Drv_Uart_WriteBlocking(const DrvUartPort *uart, const uint8_t *data, uint16_t len)
{
    if ((uart == 0) || (uart->inst == 0) || (data == 0)) {
        return;
    }

    for (uint16_t i = 0; i < len; ++i) {
        while (DL_UART_Main_isBusy(uart->inst)) {
        }
        DL_UART_Main_transmitData(uart->inst, data[i]);
    }
}

uint8_t Drv_Uart_ReadIrqByte(const DrvUartPort *uart, uint8_t *byte)
{
    if ((uart == 0) || (uart->inst == 0) || (byte == 0)) {
        return 0u;
    }

    if (DL_UART_Main_getPendingInterrupt(uart->inst) != DL_UART_MAIN_IIDX_RX) {
        return 0u;
    }

    *byte = DL_UART_Main_receiveData(uart->inst);
    return 1u;
}
