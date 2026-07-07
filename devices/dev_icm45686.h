#ifndef DEV_ICM45686_H
#define DEV_ICM45686_H

#include <stdint.h>
#include "../drivers/drv_i2c.h"
#include "imu/inv_imu_driver.h"

#define DEV_ICM45686_DEFAULT_ADDR          0x68u
#define DEV_ICM45686_DEFAULT_ACCEL_FSR_G   16.0f
#define DEV_ICM45686_DEFAULT_GYRO_FSR_DPS  2000.0f
#define DEV_ICM45686_DEFAULT_ODR_HZ        100u

typedef struct {
    DrvI2cBus bus;
    uint8_t address;
    uint8_t ready;

    inv_imu_device_t tdk;

    float accel_fsr_g;
    float gyro_fsr_dps;
    float filter_dt_s;
    float filter_alpha;

    float ax_g;
    float ay_g;
    float az_g;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    float temp_c;
    float pitch_deg;
    float roll_deg;
    float yaw_deg;
} DevIcm45686;

void Dev_Icm45686_Init(DevIcm45686 *dev, const DrvI2cBus *bus, uint8_t address);
void Dev_Icm45686_SetFilter(DevIcm45686 *dev, float dt_s, float alpha);
int Dev_Icm45686_Begin(DevIcm45686 *dev);
int Dev_Icm45686_ReadWhoAmI(DevIcm45686 *dev, uint8_t *whoami);
int Dev_Icm45686_Update(DevIcm45686 *dev);
uint8_t Dev_Icm45686_IsReady(const DevIcm45686 *dev);

#endif
