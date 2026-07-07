# Architecture

The template uses five layers.

```text
app
middleware
devices
bsp
drivers
```

## drivers

Small wrappers over MSPM0 DriverLib or simple polling logic. They know about
peripheral registers, not contest behavior.

Examples: GPIO, PWM, UART, I2C, encoder polling, line-sensor GPIO reads.

## bsp

Board support binds logical hardware to physical pins from `pin_map.h`.

Examples: left/right motor instances, the IMU I2C bus, debug UART, line-sensor
channel count.

## devices

Virtual devices hide real hardware details behind stable APIs.

Examples:

- `DevMotor`: signed motor duty and encoder state
- `DevLineSensor`: 7/8-channel line position, contrast, on-line state
- `DevIcm45686`: ICM45686/ICM45688-class IMU sample conversion
- `DevVision`: small UART packet parser for future vision targets

## middleware

Reusable logic above devices and BSP.

Examples: PID, chassis odometry/control, line tracking, telemetry.

## app

Owns the state machine, FreeRTOS tasks, and contest strategy. The template app
contains only a conservative run/stop skeleton and one basic line-following
example.

```text
app_main.c     init and binding
app_tasks.c    FreeRTOS scheduling
app_state.c    state transitions
app_mission.c  H1/H2/H3 strategy hook
app_shared.c   runtime snapshots
```
