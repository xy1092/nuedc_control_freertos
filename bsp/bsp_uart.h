#ifndef BSP_UART_H
#define BSP_UART_H

#include <stdint.h>

typedef void (*uart_rx_cb_t)(uint8_t byte);

void   BSP_Uart_Init(void);
void   BSP_Uart_SetRxCallback(uint8_t is_openmv, uart_rx_cb_t cb);
void   BSP_Uart_Send(uint8_t is_openmv, const uint8_t *data, uint16_t len);
void   BSP_Uart_Printf(const char *fmt, ...);   /* 调试串口 */

#endif
