# IMU Capture

`imu_capture.py` receives `$I` telemetry lines and writes a CSV plus a summary
for static and dynamic gyro tests.

## Firmware Commands Used

The script sends these commands by default:

```text
$RATE,<hz>
$RAWLINE,0
$PAUSE
$RAWIMU,1
```

`$RAWIMU,1` keeps the IMU stream active even when normal PID telemetry is
paused.

## Static Test

Keep the car still and capture at least 30-60 seconds:

```bash
python3 tools/imu/imu_capture.py --port /dev/ttyACM0 --mode static --duration 60 --rate 100
```

Important summary fields:

- `gx/gy/gz_mean_dps`: zero-rate bias
- `gx/gy/gz_std_dps`: gyro noise
- `yaw_delta_deg` and `gz_integral_deg`: drift over the capture window
- `accel_norm_mean_g`: should be close to 1 g while still

## Dynamic Test

Rotate the car by a known angle or move it through the expected contest motion:

```bash
python3 tools/imu/imu_capture.py --port /dev/ttyACM0 --mode dynamic --duration 20 --rate 100
```

Compare `gz_integral_deg` and `yaw_delta_deg` with the known turn angle.
