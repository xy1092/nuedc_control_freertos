#ifndef DRV_SPI_H
#define DRV_SPI_H

#include <stdint.h>
#include <ti/driverlib/driverlib.h>

#define DRV_SPI_DEFAULT_WAIT_LIMIT 100000u

typedef struct {
    SPI_Regs *inst;
    GPIO_Regs *cs_port;
    uint32_t cs_pin;
    uint32_t wait_limit;
} DrvSpiBus;

void Drv_Spi_Deselect(const DrvSpiBus *bus);
int Drv_Spi_WriteReg8(const DrvSpiBus *bus, uint8_t reg,
                      const uint8_t *data, uint32_t len);
int Drv_Spi_ReadReg8(const DrvSpiBus *bus, uint8_t reg,
                     uint8_t *data, uint32_t len);

#endif
