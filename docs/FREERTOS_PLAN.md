# FreeRTOS Runtime

All application tasks use static task control blocks and stack buffers.

```text
health   p5  20 ms       heartbeat validation and sole WWDT feed owner
imu      p4  DRDY/5 ms   initialization, calibration and snapshot publishing
fast     p4  5 ms        encoders, motor timeout and snapshot consumption
control  p3  10 ms       state machine, mission and chassis control
telem    p2  10 ms       UART commands and bounded telemetry output
```

Periodic tasks use `vTaskDelayUntil`. The IMU task waits for latched DRDY with a
50 ms finite timeout and yields for one tick while waiting.

## Recovery Levels

1. A transport operation times out and returns an error.
2. The owning module retries or reinitializes its peripheral.
3. A persistent module error raises a system fault and stops motors.
4. The health task withholds the watchdog feed.
5. WWDT0 resets the MCU within 500 ms.

Startup has a two-second heartbeat grace period. The IMU task continues to beat
during its 200 + 2000 sample static calibration, so calibration does not look
like a dead task.
