# ICM45688 Porting

## Active SPI Wiring

| ICM45688 signal | MSPM0G3507 | Configuration |
| --- | --- | --- |
| MISO | PB7 | SPI1 POCI |
| MOSI | PB8 | SPI1 PICO |
| SCLK | PB9 | SPI1 SCLK |
| CS | PB6 | manual active-low GPIO |
| INT1 | PA13 | latched active-high DRDY |

SPI1 runs Motorola mode 3, MSB first, 8-bit words at 2 MHz. The generic
platform transport is `platform/mspm0g3507/drivers/drv_spi.c`; the IMU module
owns the bus and chip select through `bsp_imu.c`.

The SPI port uses the source driver's read/write command bits, 2 us chip-select
timing, `UI_SPI4`, and the original SPI pad-slew initialization. Sensor
registers, 200 Hz timing, filters, attitude calculation and calibration values
are unchanged from `xy1092/mspm0-bmi088-icm45688`.

When moving the IMU to another SPI instance, change SysConfig and `pin_map.h`.
Do not place board pins inside `dev_icm45688.c` and do not change
`imu_profile.c` as part of a transport port.
