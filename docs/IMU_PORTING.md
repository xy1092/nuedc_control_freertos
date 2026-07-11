# ICM45688 Porting

## Active I2C Wiring

The control-board template already allocates a non-conflicting I2C bus:

| ICM45688 signal | MSPM0G3507 | Configuration |
| --- | --- | --- |
| SDA | PA16 | I2C1 SDA, 400 kHz |
| SCL | PA17 | I2C1 SCL |
| INT1 | PA15 | latched active-high DRDY |
| AD0 | module strap | address 0x68 |

This changes only the transport binding. Sensor registers, sample timing,
filtering, attitude math and calibration parameters are retained from the
source project.

## Pending SPI Wiring

The source demonstration used PB7/PB8/PB9/PB10/PB13. Those pins conflict with
the current car's buzzer and motor direction outputs, so they are not copied
into SysConfig. Until the board pin drawing is supplied, the SPI alternative is:

| ICM45688 signal | Placeholder |
| --- | --- |
| AD0/MISO | PBxx |
| SDA/MOSI | PBxx |
| SCL/SCLK | PBxx |
| CS | PAxx |
| INT1 | PAxx |

When real pins are known, add a platform SPI driver and switch only the IMU BSP
binding. Do not change `dev_icm45688.c`, `imu_profile.c` or calibration math.
