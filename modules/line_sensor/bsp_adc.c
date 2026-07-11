#include "bsp_adc.h"

#define LINE_LOW_VALUE   0u
#define LINE_HIGH_VALUE  4095u

#ifndef BSP_LINE_SENSOR_ACTIVE_LOW
#define BSP_LINE_SENSOR_ACTIVE_LOW 0
#endif

static const DrvLineSensorArray s_line_sensors = {
    BSP_ADC_CCD_COUNT,
    LINE_HIGH_VALUE,
    LINE_LOW_VALUE,
    {
        { LINE_CCD0_PORT, LINE_CCD0_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD1_PORT, LINE_CCD1_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD2_PORT, LINE_CCD2_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD3_PORT, LINE_CCD3_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD4_PORT, LINE_CCD4_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD5_PORT, LINE_CCD5_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
        { LINE_CCD6_PORT, LINE_CCD6_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
#ifdef LINE_CCD7_PIN
        { LINE_CCD7_PORT, LINE_CCD7_PIN, BSP_LINE_SENSOR_ACTIVE_LOW },
#endif
    },
};

static DevLineSensor s_line_sensor;

static const DevLineSensorConfig s_line_sensor_config = {
    &s_line_sensors,
    0,
    0,
    BSP_ADC_CCD_COUNT,
    DEV_LINE_SENSOR_DEFAULT_CONTRAST_TH,
    DEV_LINE_SENSOR_DEFAULT_STRENGTH_TH,
    DEV_LINE_SENSOR_DEFAULT_BIAS_ALPHA,
};

void BSP_Adc_Init(void)
{
    Dev_LineSensor_Init(&s_line_sensor, &s_line_sensor_config);
}

uint16_t BSP_Adc_GetBattery(void)
{
    return 0u;
}

uint16_t BSP_Adc_GetBatteryMv(void)
{
    return 0u;
}

void BSP_Adc_GetCCD(uint16_t v[BSP_ADC_CCD_COUNT])
{
    const DevLineSensorFrame *frame = Dev_LineSensor_Update(&s_line_sensor);

    if (v == 0 || frame == 0) {
        return;
    }

    for (uint8_t i = 0; i < BSP_ADC_CCD_COUNT; ++i) {
        v[i] = frame->raw[i];
    }
}

DevLineSensor *BSP_Adc_GetLineSensor(void)
{
    return &s_line_sensor;
}
