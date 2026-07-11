#include "drv_spi.h"

#define DRV_SPI_READ_BIT   0x80u
#define DRV_SPI_WRITE_MASK 0x7Fu

static uint32_t wait_limit(const DrvSpiBus *bus)
{
    return (bus->wait_limit != 0u) ? bus->wait_limit :
                                     DRV_SPI_DEFAULT_WAIT_LIMIT;
}

static void cs_delay(void)
{
#ifdef CPUCLK_FREQ
    delay_cycles(CPUCLK_FREQ / 500000u);
#else
    delay_cycles(64u);
#endif
}

static void select(const DrvSpiBus *bus)
{
    DL_GPIO_clearPins(bus->cs_port, bus->cs_pin);
    cs_delay();
}

void Drv_Spi_Deselect(const DrvSpiBus *bus)
{
    if (bus == 0 || bus->cs_port == 0 || bus->cs_pin == 0u) {
        return;
    }
    DL_GPIO_setPins(bus->cs_port, bus->cs_pin);
    cs_delay();
}

static int wait_not_busy(const DrvSpiBus *bus)
{
    uint32_t timeout = wait_limit(bus);

    while (DL_SPI_isBusy(bus->inst)) {
        if (timeout-- == 0u) {
            return -1;
        }
    }
    return 0;
}

static void drain_rx(const DrvSpiBus *bus)
{
    while (!DL_SPI_isRXFIFOEmpty(bus->inst)) {
        (void)DL_SPI_receiveData8(bus->inst);
    }
}

static int transfer8(const DrvSpiBus *bus, uint8_t tx, uint8_t *rx)
{
    uint32_t timeout = wait_limit(bus);

    while (DL_SPI_isTXFIFOFull(bus->inst)) {
        if (timeout-- == 0u) {
            return -1;
        }
    }
    DL_SPI_transmitData8(bus->inst, tx);

    timeout = wait_limit(bus);
    while (DL_SPI_isRXFIFOEmpty(bus->inst)) {
        if (timeout-- == 0u) {
            return -1;
        }
    }

    if (rx != 0) {
        *rx = DL_SPI_receiveData8(bus->inst);
    } else {
        (void)DL_SPI_receiveData8(bus->inst);
    }
    return 0;
}

static int valid(const DrvSpiBus *bus, const void *data, uint32_t len)
{
    return (bus != 0 && bus->inst != 0 && bus->cs_port != 0 &&
            bus->cs_pin != 0u && data != 0 && len != 0u) ? 1 : 0;
}

int Drv_Spi_WriteReg8(const DrvSpiBus *bus, uint8_t reg,
                      const uint8_t *data, uint32_t len)
{
    int status = -1;

    if (!valid(bus, data, len) || wait_not_busy(bus) != 0) {
        return -1;
    }
    drain_rx(bus);
    select(bus);

    if (transfer8(bus, (uint8_t)(reg & DRV_SPI_WRITE_MASK), 0) == 0) {
        status = 0;
        for (uint32_t i = 0u; i < len; ++i) {
            if (transfer8(bus, data[i], 0) != 0) {
                status = -1;
                break;
            }
        }
        if (status == 0) {
            status = wait_not_busy(bus);
        }
    }

    Drv_Spi_Deselect(bus);
    return status;
}

int Drv_Spi_ReadReg8(const DrvSpiBus *bus, uint8_t reg,
                     uint8_t *data, uint32_t len)
{
    int status = -1;

    if (!valid(bus, data, len) || wait_not_busy(bus) != 0) {
        return -1;
    }
    drain_rx(bus);
    select(bus);

    if (transfer8(bus, (uint8_t)(reg | DRV_SPI_READ_BIT), 0) == 0) {
        status = 0;
        for (uint32_t i = 0u; i < len; ++i) {
            if (transfer8(bus, 0x00u, &data[i]) != 0) {
                status = -1;
                break;
            }
        }
        if (status == 0) {
            status = wait_not_busy(bus);
        }
    }

    Drv_Spi_Deselect(bus);
    return status;
}
