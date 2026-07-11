# NUEDC Control FreeRTOS

MSPM0G3507 modular electric-contest car firmware. The project combines the
car-control template with the calibrated ICM45688 implementation from
`xy1092/mspm0-bmi088-icm45688`.

## Architecture

```text
app/                    task composition, state machine, contest mission
control/                chassis, line tracking and control algorithms
common/                 PID, filters and shared algorithms
modules/                vertical hardware modules with public service APIs
  imu/                   ICM45688 service, profile, BSP and device code
  motor/                 motor device and board binding
  encoder/               wheel encoder board binding
  line_sensor/           digital line sensor module
  telemetry/             UART command and telemetry module
  vision/                optional K230/OpenMV module
platform/mspm0g3507/     DriverLib wrappers, pin map and SysConfig
system/                  health monitor, fault manager and watchdog
third_party/             vendor sources
```

Dependencies point downward:

```text
app -> control/module public API -> device -> platform driver -> DriverLib
```

No application or control file may use SysConfig pin macros directly. Board
changes belong in `platform/mspm0g3507/`.

## FreeRTOS Reliability

| Task | Period/source | Priority | Health deadline |
| --- | ---: | ---: | ---: |
| health | 20 ms | 5 | owns WWDT feed |
| imu | ICM45688 DRDY, 200 Hz | 4 | 100 ms |
| fast | 5 ms | 4 | 50 ms |
| control | 10 ms | 3 | 100 ms |
| telemetry | 10 ms | 2 | 500 ms |

Tasks are statically allocated. WWDT0 has a 500 ms period and is fed only when
all task heartbeats are current and no system fault is active. A stale task
immediately stops both motors and prevents further watchdog feeds. The motor
module also stops non-zero output when commands have not been refreshed for
100 ms.

## ICM45688

The sensor algorithm is taken from the calibrated source project. The device
code retains its 200 Hz configuration, Mahony implementation, sample filters,
stationary yaw lock, bias learning and dynamic calibration math. Calibrated
application values are isolated in `modules/imu/imu_profile.c`:

- sample period `0.005 s`, complementary alpha `0.98`
- accelerometer `+/-16 g`, gyroscope `+/-1000 dps`
- hardware bandwidth ODR/4 and 30 Hz accelerometer sample LPF
- stationary thresholds `1.5 dps`, `0.08 g`, hold `5` samples
- acceleration trust window `0.15 g`, rejection angle `10 deg`
- Mahony gains `Kp=1.0`, `Ki=0.0`
- static calibration: 200 warm-up + 2000 samples, stddev limit `0.50 dps`
- bias learning rate `0.002`
- calibrated Z scale `0.994821`

The service owns the sensor and publishes an atomic `ImuSnapshot_t`. During
startup calibration the car stays in IDLE while the IMU task continues sending
health heartbeats.

The calibrated device uses SPI1 mode 3 at 2 MHz. Only the transport and board
binding differ from the source project:

| Signal | MSPM0G3507 pin |
| --- | --- |
| ICM45688 MISO | PB7 |
| ICM45688 MOSI | PB8 |
| ICM45688 SCLK | PB9 |
| ICM45688 CS | PB6 |
| ICM45688 INT1 | PA13 |

See `docs/IMU_PORTING.md` for the transport contract and `docs/PIN_MAP.md` for
the complete board assignment.

## Encoders

The left encoder uses native TIMG8 QEI on PB15/PB16. MSPM0G3507 does not offer
native QEI on TIMG6, so the fixed PB26/PB27 right-encoder pins use TIMG6 C0/C1
dual-edge capture interrupts and decode every quadrature transition in the ISR.
The 5 ms fast task only snapshots the accumulated hardware counts.

## Build

```bash
cd /home/xy/ti-workspace/projects/nuedc_control_freertos
./scripts/build.sh
```

Artifacts are written to `build-fw/nuedc_control_freertos.{out,hex,bin}`.

## VS Code and J-Link

The workspace configuration is adapted from
`xy1092/mspm0-cmake-jlink-config`. Open the repository folder in VS Code, then
use `Terminal -> Run Task`:

- `Build Main Firmware`: build the normal FreeRTOS car firmware.
- `Flash Main Firmware (J-Link)`: build and flash the normal firmware.
- `Build Line Sensor Test`: build the standalone seven-channel digital test.
- `Flash Line Sensor Test (J-Link)`: build and flash only the grayscale test.
- `Check MSPM0/J-Link Environment`: verify compiler, SDK and probe tools.

`Ctrl+Shift+B` builds the main firmware. The Run and Debug view provides
separate J-Link entries for the main firmware and line-sensor test. Firmware
selection is explicit, so building a test does not replace the main artifact.

## Commands

```text
s or $START             enter RUN after the IMU is ready
x or $STOP              stop the car
$DUTY,300,300,800       direct motor test for 800 ms
$RATE,100               telemetry rate
$RAWLINE,1              enable line sensor stream
$RAWIMU,1               enable IMU stream
$IMUCAL,START           start the original +4/-4 turn yaw calibration
$IMUCAL,CANCEL          cancel calibration and restore the previous scale
$PAUSE / $RESUME        pause/resume chassis telemetry
$DUMP                   print telemetry configuration
```

## Adding A Module

1. Add a self-contained directory under `modules/<name>/`.
2. Expose only the stable public API in one header.
3. Put MCU-independent device behavior inside the module.
4. Add pin/peripheral aliases only under `platform/mspm0g3507/`.
5. Give a stateful module one owning task and publish snapshots to consumers.
6. Register its heartbeat and finite deadline if it is safety critical.
7. Add sources and include paths explicitly in `CMakeLists.txt`.

The module must use finite communication timeouts and must define its safe
output when data or commands become stale.
