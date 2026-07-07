#include "drv_gpio.h"

void Drv_Gpio_WriteRaw(const DrvGpioPin *pin, uint8_t high)
{
    if ((pin == 0) || (pin->port == 0)) {
        return;
    }

    if (high) {
        DL_GPIO_setPins(pin->port, pin->pin);
    } else {
        DL_GPIO_clearPins(pin->port, pin->pin);
    }
}

uint8_t Drv_Gpio_ReadRaw(const DrvGpioPin *pin)
{
    if ((pin == 0) || (pin->port == 0)) {
        return 0u;
    }

    return (DL_GPIO_readPins(pin->port, pin->pin) != 0u) ? 1u : 0u;
}

void Drv_Gpio_WriteActive(const DrvGpioPin *pin, uint8_t active)
{
    uint8_t high = active ? 1u : 0u;

    if ((pin != 0) && pin->active_low) {
        high = high ? 0u : 1u;
    }

    Drv_Gpio_WriteRaw(pin, high);
}

uint8_t Drv_Gpio_ReadActive(const DrvGpioPin *pin)
{
    uint8_t high = Drv_Gpio_ReadRaw(pin);

    if ((pin != 0) && pin->active_low) {
        high = high ? 0u : 1u;
    }

    return high;
}

void Drv_Gpio_Toggle(const DrvGpioPin *pin)
{
    if ((pin == 0) || (pin->port == 0)) {
        return;
    }

    DL_GPIO_togglePins(pin->port, pin->pin);
}
