# Vehicle Calibration

The single source of truth for measured vehicle constants is
`config/vehicle_calibration.h`. Hardware tests and the production chassis use
the same definitions.

## Verified Values

| Item | Value | Verification |
| --- | ---: | --- |
| Left encoder | 73.44 pulse/cm | 100 cm straight trim |
| Right encoder | 73.14 pulse/cm | 99.8 cm measured over 100 cm |
| Straight PWM | left 300, right 300 | physical straight run |
| Encoder synchronization | Kp 0.08, deadband 3 pulses, limit 15 PWM | physical straight run |
| Distance deceleration | final 900 pulses, minimum PWM 220 | physical 100 cm run |
| Stationary yaw turn | 90 degrees, 0.2 degree final tolerance | CW and CCW button test |
| Grayscale order | PB20, PB24, PB25, PA24, PA25, PA26, PA27 | left-to-right sweep |
| Grayscale type | seven digital channels | transition test |
| Motor polarity | both positive commands move forward | wheel test |
| ICM45688 | calibrated profile in `modules/imu/imu_profile.c` | OLED yaw and turn tests |

The line-tracking PID and wheel-speed PID are initial template values, not
course-calibrated values. Tune them only after the course layout and operating
speed are fixed.

## Control Use

Production odometry converts each encoder with its own pulse-per-centimeter
value. Equal physical straight speed therefore uses a slightly higher left
pulse target than right pulse target. The stationary yaw turn constants remain
available for precise stop-and-turn actions; a continuous racing turn should
use a separate non-stopping transition controller.
