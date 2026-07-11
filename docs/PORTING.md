# Porting Checklist

1. Update `platform/mspm0g3507/nuedc_control_freertos.syscfg`.
2. Regenerate `ti_msp_dl_config.c` and `ti_msp_dl_config.h` with TI SysConfig.
3. Update logical aliases in `platform/mspm0g3507/pin_map.h`.
4. Build before changing module or control code.
5. Confirm the health task continuously feeds WWDT0.
6. Test debug UART with `$DUMP`.
7. Lift the car and test motor timeout with `$DUTY,300,300,800`.
8. Verify encoder sign and left/right mapping.
9. Enable `$RAWLINE,1` and verify sensor order.
10. Keep the car still through IMU startup calibration, then enable `$RAWIMU,1`.
11. Tune control parameters only after module bring-up is complete.

Unknown physical pins belong in documentation as `PAxx` or `PBxx`; invalid
placeholder names must never be placed in compilable SysConfig or C headers.
