#include "drv_i2c.h"

static uint32_t wait_limit(const DrvI2cBus *bus)
{
    return (bus->wait_limit != 0u) ? bus->wait_limit : DRV_I2C_DEFAULT_WAIT_LIMIT;
}

static uint8_t tx_fifo_bytes(const DrvI2cBus *bus)
{
    return (bus->tx_fifo_bytes != 0u) ? bus->tx_fifo_bytes : DRV_I2C_DEFAULT_TX_FIFO_BYTES;
}

static int valid_bus(const DrvI2cBus *bus)
{
    return ((bus != 0) && (bus->inst != 0)) ? 1 : 0;
}

int Drv_I2c_WaitIdle(const DrvI2cBus *bus)
{
    uint32_t timeout;
    uint32_t status;

    if (!valid_bus(bus)) {
        return -1;
    }

    timeout = wait_limit(bus);
    do {
        status = DL_I2C_getControllerStatus(bus->inst);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0u) {
            DL_I2C_resetControllerTransfer(bus->inst);
            return -1;
        }
        if (timeout-- == 0u) {
            DL_I2C_resetControllerTransfer(bus->inst);
            return -1;
        }
    } while ((status & DL_I2C_CONTROLLER_STATUS_IDLE) == 0u ||
             (status & (DL_I2C_CONTROLLER_STATUS_BUSY |
                        DL_I2C_CONTROLLER_STATUS_BUSY_BUS)) != 0u);

    return 0;
}

int Drv_I2c_Write(const DrvI2cBus *bus, uint8_t addr, const uint8_t *data, uint16_t len)
{
    if (!valid_bus(bus) || data == 0 || len == 0u || len > tx_fifo_bytes(bus)) {
        return -1;
    }

    DL_I2C_resetControllerTransfer(bus->inst);
    if (DL_I2C_fillControllerTXFIFO(bus->inst, data, len) != len) {
        DL_I2C_resetControllerTransfer(bus->inst);
        return -1;
    }
    if (Drv_I2c_WaitIdle(bus) != 0) {
        return -1;
    }

    DL_I2C_startControllerTransfer(bus->inst, addr,
        DL_I2C_CONTROLLER_DIRECTION_TX, len);
    delay_cycles(32u);

    return Drv_I2c_WaitIdle(bus);
}

int Drv_I2c_WriteReg8(const DrvI2cBus *bus, uint8_t addr, uint8_t reg,
                      const uint8_t *data, uint16_t len)
{
    uint8_t tx[DRV_I2C_DEFAULT_TX_FIFO_BYTES];

    if (!valid_bus(bus) || len > (uint16_t)(tx_fifo_bytes(bus) - 1u) ||
        len > (uint16_t)(sizeof(tx) - 1u)) {
        return -1;
    }

    tx[0] = reg;
    for (uint16_t i = 0; i < len; ++i) {
        tx[i + 1u] = data[i];
    }

    return Drv_I2c_Write(bus, addr, tx, (uint16_t)(len + 1u));
}

static void start_register_read(I2C_Regs *inst, uint8_t addr, uint16_t len)
{
    DL_Common_updateReg(&inst->MASTER.MSA,
        (((uint32_t)addr << I2C_MSA_SADDR_OFS) |
         (uint32_t)DL_I2C_CONTROLLER_DIRECTION_RX),
        (I2C_MSA_SADDR_MASK | I2C_MSA_DIR_MASK));

    DL_Common_updateReg(&inst->MASTER.MCTR,
        (((uint32_t)len << I2C_MCTR_MBLEN_OFS) |
         I2C_MCTR_BURSTRUN_ENABLE |
         I2C_MCTR_START_ENABLE |
         I2C_MCTR_RD_ON_TXEMPTY_ENABLE |
         I2C_MCTR_STOP_ENABLE),
        (I2C_MCTR_MBLEN_MASK |
         I2C_MCTR_BURSTRUN_MASK |
         I2C_MCTR_START_MASK |
         I2C_MCTR_RD_ON_TXEMPTY_MASK |
         I2C_MCTR_STOP_MASK));
}

int Drv_I2c_ReadReg8(const DrvI2cBus *bus, uint8_t addr, uint8_t reg,
                     uint8_t *data, uint16_t len)
{
    uint32_t timeout;
    uint16_t rx = 0u;

    if (!valid_bus(bus) || data == 0 || len == 0u || len > 0x0FFFu) {
        return -1;
    }

    DL_I2C_resetControllerTransfer(bus->inst);
    if (DL_I2C_fillControllerTXFIFO(bus->inst, &reg, 1u) != 1u) {
        DL_I2C_resetControllerTransfer(bus->inst);
        return -1;
    }
    if (Drv_I2c_WaitIdle(bus) != 0) {
        return -1;
    }

    start_register_read(bus->inst, addr, len);
    delay_cycles(32u);

    timeout = wait_limit(bus) * ((uint32_t)len + 1u);
    while (rx < len) {
        uint32_t status = DL_I2C_getControllerStatus(bus->inst);
        if ((status & DL_I2C_CONTROLLER_STATUS_ERROR) != 0u) {
            DL_I2C_resetControllerTransfer(bus->inst);
            return -1;
        }
        if (!DL_I2C_isControllerRXFIFOEmpty(bus->inst)) {
            data[rx++] = DL_I2C_receiveControllerData(bus->inst);
            timeout = wait_limit(bus) * ((uint32_t)(len - rx) + 1u);
        } else if (timeout-- == 0u) {
            DL_I2C_resetControllerTransfer(bus->inst);
            return -1;
        }
    }

    return Drv_I2c_WaitIdle(bus);
}
