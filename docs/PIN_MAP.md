# Final Pin Map

Target: MSPM0G3507, LQFP-64. This table matches
`platform/mspm0g3507/nuedc_control_freertos.syscfg`.

| Module | Signal | Pin | Peripheral / behavior |
| --- | --- | --- | --- |
| OLED | SDA | PA0 | I2C0 SDA, 400 kHz |
| OLED | SCL | PA1 | I2C0 SCL, 400 kHz |
| Grayscale | CH1 (leftmost) | PB20 | digital input, pull-up |
| Grayscale | CH2 | PB24 | digital input, pull-up |
| Grayscale | CH3 | PB25 | digital input, pull-up |
| Grayscale | CH4 | PA24 | digital input, pull-up |
| Grayscale | CH5 | PA25 | digital input, pull-up |
| Grayscale | CH6 | PA26 | digital input, pull-up |
| Grayscale | CH7 (rightmost) | PA27 | digital input, pull-up |
| Motor A | PWM | PA8 | TIMA0 C0 |
| Motor A | AIN1 | PA15 | GPIO output |
| Motor A | AIN2 | PA16 | GPIO output |
| Motor B | PWM | PA9 | TIMA0 C1 |
| Motor B | BIN1 | PA12 | GPIO output |
| Motor B | BIN2 | PB23 | GPIO output |
| Motor driver | STBY | 3V3 | tied high, no MCU pin |
| Encoder A | EA | PB15 | TIMG8 C0, native QEI |
| Encoder A | EB | PB16 | TIMG8 C1, native QEI |
| Encoder B | EA | PB26 | TIMG6 C0, dual-edge capture |
| Encoder B | EB | PB27 | TIMG6 C1, dual-edge capture |
| ICM45688 | MISO | PB7 | SPI1 POCI |
| ICM45688 | MOSI | PB8 | SPI1 PICO |
| ICM45688 | SCLK | PB9 | SPI1 SCLK |
| ICM45688 | CS | PB6 | manual active-low GPIO |
| ICM45688 | INT1 | PA13 | latched active-high DRDY |
| Control | start/stop | PB21 | active-low input, pull-up |
| Control | mode | PB1 | active-low input, pull-up |
| Control | calibration | PA7 | active-low input, pull-up |
| Buzzer | signal | PB14 | GPIO output |
| Status LED | signal | PB22 | GPIO output |
| Debug UART | TX | PA10 | UART0 TX, 115200 8N1 |
| Debug UART | RX | PA11 | UART0 RX, 115200 8N1 |

## Timer Allocation

| Peripheral | Owner |
| --- | --- |
| TIMA0 C0/C1 | two motor PWM channels |
| TIMG8 C0/C1 | left native quadrature encoder |
| TIMG6 C0/C1 | right encoder edge capture |
| WWDT0 | 500 ms system watchdog |

All grayscale channels are GPIO digital inputs. Their logical order in
`bsp_adc.c` is CH1 through CH7 in the order shown above.
