#include "bsp_imu.h"
#include "pin_map.h"

static const DrvSpiBus s_imu_spi = {
    IMU_SPI_INST,
    IMU_CS_PORT,
    IMU_CS_PIN,
    DRV_SPI_DEFAULT_WAIT_LIMIT,
};

const DrvSpiBus *BSP_IMU_GetBus(void)
{
    return &s_imu_spi;
}

uint8_t BSP_IMU_DataReady(void)
{
    return ((DL_GPIO_readPins(IMU_INT_PORT, IMU_INT_PIN) & IMU_INT_PIN) != 0u) ?
           1u : 0u;
}
