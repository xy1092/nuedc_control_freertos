#include "bsp_imu.h"
#include "../pin_map.h"
#include "../devices/dev_icm45686.h"

static const DrvI2cBus s_imu_i2c = {
    I2C_INST,
    DRV_I2C_DEFAULT_WAIT_LIMIT,
    DRV_I2C_DEFAULT_TX_FIFO_BYTES,
};

static DevIcm45686 s_imu;

void BSP_IMU_Init(void)
{
    Dev_Icm45686_Init(&s_imu, &s_imu_i2c, IMU_I2C_ADDR);
    Dev_Icm45686_SetFilter(&s_imu, 0.005f, 0.98f);
    (void)Dev_Icm45686_Begin(&s_imu);
}

void BSP_IMU_Update(float *pitch, float *roll, float *yaw)
{
    if (Dev_Icm45686_Update(&s_imu) != INV_IMU_OK) {
        return;
    }

    if (pitch) *pitch = s_imu.pitch_deg;
    if (roll) *roll = s_imu.roll_deg;
    if (yaw) *yaw = s_imu.yaw_deg;
}

void BSP_IMU_GetGyro(float *gx, float *gy, float *gz)
{
    if (gx) *gx = s_imu.gx_dps;
    if (gy) *gy = s_imu.gy_dps;
    if (gz) *gz = s_imu.gz_dps;
}

void BSP_IMU_GetAccel(float *ax, float *ay, float *az)
{
    if (ax) *ax = s_imu.ax_g;
    if (ay) *ay = s_imu.ay_g;
    if (az) *az = s_imu.az_g;
}

uint8_t BSP_IMU_IsReady(void)
{
    return Dev_Icm45686_IsReady(&s_imu);
}

uint8_t BSP_IMU_ReadWhoAmI(uint8_t *whoami)
{
    return (Dev_Icm45686_ReadWhoAmI(&s_imu, whoami) == INV_IMU_OK) ? 1u : 0u;
}
