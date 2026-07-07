#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdint.h>
#include <ti/driverlib/driverlib.h>

typedef struct {
    GPIO_Regs *port;
    uint32_t pin;
    uint8_t active_low;
} DrvGpioPin;

void Drv_Gpio_WriteRaw(const DrvGpioPin *pin, uint8_t high);
uint8_t Drv_Gpio_ReadRaw(const DrvGpioPin *pin);
void Drv_Gpio_WriteActive(const DrvGpioPin *pin, uint8_t active);
uint8_t Drv_Gpio_ReadActive(const DrvGpioPin *pin);
void Drv_Gpio_Toggle(const DrvGpioPin *pin);

#endif
