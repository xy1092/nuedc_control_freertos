#ifndef BSP_K230_H
#define BSP_K230_H

#include <stdint.h>

typedef void (*k230_rx_cb_t)(uint8_t byte);

void    BSP_K230_Init(void);
void    BSP_K230_SetRxCallback(k230_rx_cb_t cb);
void    BSP_K230_Send(const uint8_t *data, uint16_t len);
void    BSP_K230_Reset(uint8_t hold_reset);
uint8_t BSP_K230_IsReady(void);

#endif /* BSP_K230_H */
