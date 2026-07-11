# Architecture

This firmware is a modular monolith: shared platform code is horizontal, while
hardware capabilities are vertical modules.

```text
app
 |-- control
 |-- module public APIs
       |-- service/state owner
       |-- device implementation
             |-- platform drivers
                   |-- TI DriverLib
```

`app/` owns task creation, contest state and mission flow. `control/` contains
algorithms that combine module data. `modules/` owns hardware capabilities and
their state. `platform/` owns the MCU, generated names and physical pins.
`system/` owns faults, task liveness and reset policy.

Physical constants measured on the assembled car are owned by
`config/vehicle_calibration.h`. Tests and production code reference that file
instead of keeping separate copies of calibration values.

## Module Contract

Each stateful module should provide:

- one public API header;
- one task or one explicit owner of mutable hardware state;
- a timestamped snapshot for readers;
- finite transport timeouts;
- an initialization state and fault state;
- a defined safe behavior for stale input;
- a health heartbeat when safety critical.

Modules must not reach into another module's private device object. Cross-module
coordination belongs in `control/` or `app/`.

## Board Support

The board support portion of a module binds logical signals to
`platform/mspm0g3507/pin_map.h`. It may select an I2C instance, GPIO or UART,
but it must not implement filtering, calibration, route logic or UI behavior.

## IMU Ownership

`imu_service.c` is the only owner of the ICM45688 instance. It performs the
original startup and dynamic calibration flows, consumes DRDY at 200 Hz and
publishes `ImuSnapshot_t` inside a FreeRTOS critical section. Other tasks never
touch the SPI bus or TDK object directly.

## Failure Containment

The health task is the only code that feeds WWDT0. A missed heartbeat causes
the fault manager to stop motor output and withhold the feed. If scheduling or
interrupts are globally stuck, the independent 500 ms WWDT still resets the
MCU.
