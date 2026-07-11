# Standalone Hardware Tests

Hardware tests are separate CMake targets. They reuse the production board
bindings without starting the production mission application. Some tests run
their own focused FreeRTOS control task.

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

## Button IMU Turn Coordination

The FreeRTOS coordination test uses the production IMU service, calibrated
profile, motor BSP, health monitor and watchdog. Keep the car still during IMU
calibration, then use one button at a time:

| Button | Pin | Action |
| --- | --- | --- |
| Calibration | PA7 | clockwise 90 degrees |
| Mode | PB1 | counterclockwise 90 degrees |
| Start/stop | PB21 | clockwise 90, pause, then counterclockwise 90 |

Build or flash from VS Code with `Build IMU Turn Test` and
`Flash IMU Turn Test (J-Link)`. OLED and UART0 report the current phase, yaw,
target, error and motor command. The controller brakes near the target and uses
adaptive fine pulses until the stationary yaw error is within 0.2 degrees. A
wrong initial direction, stale IMU, timeout or task-health fault stops both
motors.

## 100 cm Straight Encoder Calibration

This FreeRTOS test uses requirement 1 of the H-problem field: A to B is a
100 cm straight segment. Put digital grayscale channel 4 over the black arc at
A, point the car toward B and briefly press PB21. The grayscale input records
leaving A and whether any channel crosses B; both encoders independently close
their wheel's distance loop.

The right encoder calibration is `73.14 pulse/cm`. Straight-line trim currently
uses `73.44 pulse/cm` for the left encoder, giving 100 cm targets of 7344 left
and 7314 right. Each wheel decelerates independently over its final 900 edges
and brakes when its own target is reached. The open-loop PWM values are trimmed
to 300/300. A limited encoder-ratio correction keeps both wheel distances
synchronized while running. Yaw is telemetry only and is not used for steering.
OLED retains both pulse counts, both converted distances, and status:

| Status | Meaning |
| --- | --- |
| `S:1` | Reached the encoder target without observing B |
| `S:3` | Reached the encoder target and observed B |
| `S:-2` | 15 second timeout |

The calibrated right-wheel run measured 99.8 cm over the physical 100 cm
course. The final `73.44/73.14 pulse/cm` left/right ratio was physically
verified to run straight. These values are shared with the production chassis
through `config/vehicle_calibration.h`.

Build or flash with `Build Motor Encoder IMU Test` and
`Flash Motor Encoder IMU Test (J-Link)`.
