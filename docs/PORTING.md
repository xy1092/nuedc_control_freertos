# Porting Checklist

Use this order when moving the template to a new board or car.

1. Update `nuedc_control_template.syscfg`.
2. Update aliases in `pin_map.h`.
3. Build once before touching app logic.
4. Test debug UART with `$DUMP`.
5. Lift the car and test motors with `$DUTY,300,300,800`.
6. Verify encoder sign and left/right mapping.
7. Enable `$RAWLINE,1` and check logical line order is left-to-right.
8. Enable `$RAWIMU,1` and run `tools/imu/imu_capture.py`.
9. Only then tune PID and app strategy.

Expected bring-up commands:

```text
$DUMP
$DUTY,300,300,800
$RAWLINE,1
$RAWIMU,1
```
