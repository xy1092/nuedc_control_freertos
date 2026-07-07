#include "dev_icm45686.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifndef DEV_ICM45686_CPU_HZ
#ifdef CPUCLK_FREQ
#define DEV_ICM45686_CPU_HZ CPUCLK_FREQ
#else
#define DEV_ICM45686_CPU_HZ 32000000u
#endif
#endif

#define DEV_ICM45686_RAW_MAX 32768.0f

static void sleep_us(uint32_t us)
{
    uint32_t cycles_per_us = DEV_ICM45686_CPU_HZ / 1000000u;

    if (cycles_per_us == 0u) {
        cycles_per_us = 1u;
    }

    while (us > 1000u) {
        delay_cycles(cycles_per_us * 1000u);
        us -= 1000u;
    }
    if (us > 0u) {
        delay_cycles(cycles_per_us * us);
    }
}

static int transport_write(void *context, uint8_t reg, const uint8_t *buf, uint32_t len)
{
    DevIcm45686 *dev = (DevIcm45686 *)context;

    if (dev == NULL || len > UINT16_MAX) {
        return -1;
    }

    return Drv_I2c_WriteReg8(&dev->bus, dev->address, reg, buf, (uint16_t)len);
}

static int transport_read(void *context, uint8_t reg, uint8_t *buf, uint32_t len)
{
    DevIcm45686 *dev = (DevIcm45686 *)context;

    if (dev == NULL || len > 0x0FFFu) {
        return -1;
    }

    return Drv_I2c_ReadReg8(&dev->bus, dev->address, reg, buf, (uint16_t)len);
}

static void clear_measurements(DevIcm45686 *dev)
{
    dev->ax_g = 0.0f;
    dev->ay_g = 0.0f;
    dev->az_g = 0.0f;
    dev->gx_dps = 0.0f;
    dev->gy_dps = 0.0f;
    dev->gz_dps = 0.0f;
    dev->temp_c = 0.0f;
    dev->pitch_deg = 0.0f;
    dev->roll_deg = 0.0f;
    dev->yaw_deg = 0.0f;
}

void Dev_Icm45686_Init(DevIcm45686 *dev, const DrvI2cBus *bus, uint8_t address)
{
    if (dev == NULL) {
        return;
    }

    memset(dev, 0, sizeof(*dev));
    if (bus != NULL) {
        dev->bus = *bus;
    }
    dev->address = (address == 0u) ? DEV_ICM45686_DEFAULT_ADDR : address;
    dev->accel_fsr_g = DEV_ICM45686_DEFAULT_ACCEL_FSR_G;
    dev->gyro_fsr_dps = DEV_ICM45686_DEFAULT_GYRO_FSR_DPS;
    dev->filter_dt_s = 1.0f / (float)DEV_ICM45686_DEFAULT_ODR_HZ;
    dev->filter_alpha = 0.98f;

    dev->tdk.transport.context = dev;
    dev->tdk.transport.read_reg = transport_read;
    dev->tdk.transport.write_reg = transport_write;
    dev->tdk.transport.serif_type = UI_I2C;
    dev->tdk.transport.sleep_us = sleep_us;
}

void Dev_Icm45686_SetFilter(DevIcm45686 *dev, float dt_s, float alpha)
{
    if (dev == NULL) {
        return;
    }
    if (dt_s > 0.0f) {
        dev->filter_dt_s = dt_s;
    }
    if (alpha >= 0.0f && alpha <= 1.0f) {
        dev->filter_alpha = alpha;
    }
}

int Dev_Icm45686_ReadWhoAmI(DevIcm45686 *dev, uint8_t *whoami)
{
    if (dev == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }

    return inv_imu_get_who_am_i(&dev->tdk, whoami);
}

int Dev_Icm45686_Begin(DevIcm45686 *dev)
{
    int status = INV_IMU_OK;
    uint8_t who = 0u;

    if (dev == NULL || dev->bus.inst == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }

    dev->ready = 0u;
    clear_measurements(dev);
    sleep_us(3000u);

    status |= Dev_Icm45686_ReadWhoAmI(dev, &who);
    if (status != INV_IMU_OK || who != INV_IMU_WHOAMI) {
        return INV_IMU_ERROR;
    }

    status |= inv_imu_soft_reset(&dev->tdk);
    status |= inv_imu_set_accel_fsr(&dev->tdk, ACCEL_CONFIG0_ACCEL_UI_FS_SEL_16_G);
    status |= inv_imu_set_accel_frequency(&dev->tdk, ACCEL_CONFIG0_ACCEL_ODR_100_HZ);
    status |= inv_imu_set_gyro_fsr(&dev->tdk, GYRO_CONFIG0_GYRO_UI_FS_SEL_2000_DPS);
    status |= inv_imu_set_gyro_frequency(&dev->tdk, GYRO_CONFIG0_GYRO_ODR_100_HZ);
    status |= inv_imu_set_accel_mode(&dev->tdk, PWR_MGMT0_ACCEL_MODE_LN);
    status |= inv_imu_set_gyro_mode(&dev->tdk, PWR_MGMT0_GYRO_MODE_LN);
    if (status != INV_IMU_OK) {
        return status;
    }

    sleep_us(GYR_STARTUP_TIME_US);
    dev->ready = 1u;
    return INV_IMU_OK;
}

int Dev_Icm45686_Update(DevIcm45686 *dev)
{
    inv_imu_sensor_data_t raw;
    int status;

    if (dev == NULL) {
        return INV_IMU_ERROR_BAD_ARG;
    }
    if (!dev->ready) {
        return INV_IMU_ERROR;
    }

    status = inv_imu_get_register_data(&dev->tdk, &raw);
    if (status != INV_IMU_OK) {
        return status;
    }

    dev->ax_g = ((float)raw.accel_data[0] * dev->accel_fsr_g) / DEV_ICM45686_RAW_MAX;
    dev->ay_g = ((float)raw.accel_data[1] * dev->accel_fsr_g) / DEV_ICM45686_RAW_MAX;
    dev->az_g = ((float)raw.accel_data[2] * dev->accel_fsr_g) / DEV_ICM45686_RAW_MAX;
    dev->gx_dps = ((float)raw.gyro_data[0] * dev->gyro_fsr_dps) / DEV_ICM45686_RAW_MAX;
    dev->gy_dps = ((float)raw.gyro_data[1] * dev->gyro_fsr_dps) / DEV_ICM45686_RAW_MAX;
    dev->gz_dps = ((float)raw.gyro_data[2] * dev->gyro_fsr_dps) / DEV_ICM45686_RAW_MAX;
    dev->temp_c = 25.0f + ((float)raw.temp_data / 128.0f);

    float acc_pitch = atan2f(dev->ay_g, dev->az_g) * 57.2957795f;
    float acc_roll = atan2f(-dev->ax_g,
                            sqrtf(dev->ay_g * dev->ay_g +
                                  dev->az_g * dev->az_g)) * 57.2957795f;
    float alpha = dev->filter_alpha;

    dev->pitch_deg = alpha * (dev->pitch_deg + dev->gx_dps * dev->filter_dt_s) +
                     (1.0f - alpha) * acc_pitch;
    dev->roll_deg = alpha * (dev->roll_deg + dev->gy_dps * dev->filter_dt_s) +
                    (1.0f - alpha) * acc_roll;
    dev->yaw_deg += dev->gz_dps * dev->filter_dt_s;

    return INV_IMU_OK;
}

uint8_t Dev_Icm45686_IsReady(const DevIcm45686 *dev)
{
    return (dev != NULL) ? dev->ready : 0u;
}
