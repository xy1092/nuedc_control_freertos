#include "bsp_imu.h"
#include "pin_map.h"

static const DrvI2cBus s_imu_i2c = {
    I2C_INST,
    DRV_I2C_DEFAULT_WAIT_LIMIT,
    DRV_I2C_DEFAULT_TX_FIFO_BYTES,
};

const DrvI2cBus *BSP_IMU_GetBus(void)
{
    return &s_imu_i2c;
}

uint8_t BSP_IMU_DataReady(void)
{
    return ((DL_GPIO_readPins(IMU_INT_PORT, IMU_INT_PIN) & IMU_INT_PIN) != 0u) ?
           1u : 0u;
}
