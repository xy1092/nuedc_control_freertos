# NUEDC Control Template

Clean MSPM0G3507 electric-contest car firmware template.

This repository is intentionally separate from any finished car project. It is
a reusable starting point with five code layers:

```text
drivers/     low-level peripheral wrappers
bsp/         board support and pin binding
devices/     virtual devices such as motors, line sensor, IMU, vision
middleware/  PID, filters, chassis control, telemetry
app/         FreeRTOS tasks and contest strategy hooks
```

FreeRTOS is the scheduler, not a sixth architecture layer. The app layer is
split so task scheduling, state transitions, and contest strategy stay
separate:

```text
app_main.c     hardware/module init and telemetry binding
app_tasks.c    FreeRTOS task bodies and task creation
app_state.c    IDLE/RUN/FAULT state transitions and command handling
app_mission.c  contest strategy hook; default is a basic line-follow example
app_shared.c   cross-task snapshots and shared runtime state
```

The default app creates three tasks:

```text
fast      5 ms   encoder, IMU, fast sensor sampling
ctrl     10 ms   state machine and chassis/line control
telem    10 ms   serial telemetry and command handling
```

## Hardware Interfaces

- 7/8-channel digital line sensor
- A/B H-bridge motor outputs
- quadrature encoder interface
- ICM45686/ICM45688-class IMU over I2C
- optional K230/OpenMV-style vision UART
- debug UART telemetry at 115200 baud

Pin changes should be made in `nuedc_control_template.syscfg` and `pin_map.h`.
Upper layers should use BSP/device interfaces instead of SysConfig macros.

## Build

```bash
cd /home/xy/ti-workspace/projects/nuedc_control_template
./scripts/build.sh
```

Output files are placed under `build-fw/`.

## Serial Commands

```text
s or $START          enter run state
x or $STOP           stop motors
$DUTY,300,300,800    direct motor PWM test for 800 ms
$RATE,100            telemetry rate in Hz
$RAWLINE,1           enable raw line-sensor stream
$RAWIMU,1            enable IMU stream
$PAUSE/$RESUME       pause/resume normal chassis telemetry
$DUMP                print telemetry flags
```

The IMU stream is:

```text
$I,<ts_ms>,<pitch_deg>,<roll_deg>,<yaw_deg>,<gx_dps>,<gy_dps>,<gz_dps>,<ax_g>,<ay_g>,<az_g>,<ready>
```

## IMU Static/Dynamic Capture

```bash
python3 tools/imu/imu_capture.py --port /dev/ttyACM0 --mode static --duration 60 --rate 100
python3 tools/imu/imu_capture.py --port /dev/ttyACM0 --mode dynamic --duration 20 --rate 100
```

Logs are written to `tools/imu/logs/`.

## Where To Add Contest Logic

Start in `app/app_mission.c`. If a strategy becomes reusable or algorithmic,
move that algorithm into `middleware/control/` and keep `app_mission.c` as the
state-machine owner. Do not put route logic into BSP or drivers. BSP should
only bind physical pins and peripherals to virtual devices.
