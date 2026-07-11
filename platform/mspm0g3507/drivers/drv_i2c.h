#ifndef DRV_I2C_H
#define DRV_I2C_H

#include <stdint.h>
#include <ti/driverlib/driverlib.h>

#define DRV_I2C_DEFAULT_WAIT_LIMIT 100000u
#define DRV_I2C_DEFAULT_TX_FIFO_BYTES 8u

typedef struct {
    I2C_Regs *inst;
    uint32_t wait_limit;
    uint8_t tx_fifo_bytes;
} DrvI2cBus;

int Drv_I2c_WaitIdle(const DrvI2cBus *bus);
int Drv_I2c_Write(const DrvI2cBus *bus, uint8_t addr, const uint8_t *data, uint16_t len);
int Drv_I2c_WriteReg8(const DrvI2cBus *bus, uint8_t addr, uint8_t reg,
                      const uint8_t *data, uint16_t len);
int Drv_I2c_ReadReg8(const DrvI2cBus *bus, uint8_t addr, uint8_t reg,
                     uint8_t *data, uint16_t len);

#endif
