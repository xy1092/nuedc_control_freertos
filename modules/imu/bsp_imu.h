#ifndef BSP_IMU_H
#define BSP_IMU_H

#include <stdint.h>
#include "drv_i2c.h"

const DrvI2cBus *BSP_IMU_GetBus(void);
uint8_t BSP_IMU_DataReady(void);

#endif
