# Standalone Hardware Tests

Hardware tests are separate CMake targets. They reuse the production board
bindings but do not start the FreeRTOS application or control algorithms.

## Digital Grayscale Sensor

Build and flash from VS Code with:

- `Build Line Sensor Test`
- `Flash Line Sensor Test (J-Link)`

The test samples CH1 through CH7 every 10 ms and reports every 100 ms through
UART0 at 115200 baud and SEGGER RTT. PB22 toggles every 500 ms as a heartbeat.

```text
$GRAY,ms,ch1,ch2,ch3,ch4,ch5,ch6,ch7,bits,changed,seen0,seen1,edge1,...,edge7
```

`seen0` and `seen1` are bit masks accumulated since reset. After each sensor
has crossed both black and white, both masks should read `0x7F`. The edge
counters expose disconnected, stuck or noisy channels. The electrical meaning
of 0 and 1 is intentionally not assumed until the physical black/white test is
complete.

## Motors And Buttons

Lift the driven wheels before flashing. Build and flash from VS Code with:

- `Build Motor Test`
- `Flash Motor Test (J-Link)`

All buttons are active-low. Hold exactly one button to run the selected test at
300/1000 duty; release it to coast both motors. Holding multiple buttons also
stops both motors.

| Button | Pin | Action |
| --- | --- | --- |
| Calibration | PA7 | left motor forward |
| Mode | PB1 | right motor forward |
| Start/stop | PB21 | both motors forward |

UART0 reports state changes at 115200 baud as `$MOTOR,state,left,right`.

## OLED And ICM45688 Yaw

This FreeRTOS test reuses the calibrated ICM45688 profile and the SSD1306
driver from `mspm0-bmi088-icm45688`. Build and flash from VS Code with:

- `Build OLED IMU Yaw Test`
- `Flash OLED IMU Yaw Test (J-Link)`

Connect a 128x64 SSD1306-compatible OLED to PA0 (SDA) and PA1 (SCL). Addresses
0x3C and 0x3D are detected automatically. Keep the IMU still while the display
shows `CALIBRATING / KEEP STILL`. After calibration, the current continuous
`yaw_deg` value is refreshed every 100 ms. UART0 also reports signed
millidegrees as `$YAW,value` at 115200 baud.
