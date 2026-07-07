#ifndef BSP_IMU_H
#define BSP_IMU_H

#include <stdint.h>

void BSP_IMU_Init(void);
void BSP_IMU_Update(float *pitch, float *roll, float *yaw);
void BSP_IMU_GetGyro(float *gx, float *gy, float *gz);
void BSP_IMU_GetAccel(float *ax, float *ay, float *az);
uint8_t BSP_IMU_IsReady(void);
uint8_t BSP_IMU_ReadWhoAmI(uint8_t *whoami);

#endif
