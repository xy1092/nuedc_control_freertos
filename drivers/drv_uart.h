#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdint.h>
#include <ti/driverlib/driverlib.h>

typedef struct {
    UART_Regs *inst;
    IRQn_Type irqn;
} DrvUartPort;

void Drv_Uart_EnableRxInterrupt(const DrvUartPort *uart);
void Drv_Uart_WriteBlocking(const DrvUartPort *uart, const uint8_t *data, uint16_t len);
uint8_t Drv_Uart_ReadIrqByte(const DrvUartPort *uart, uint8_t *byte);

#endif
