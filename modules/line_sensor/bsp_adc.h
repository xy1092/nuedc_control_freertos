#ifndef BSP_ADC_H
#define BSP_ADC_H

#include <stdint.h>
#include "pin_map.h"
#include "dev_line_sensor.h"

#ifdef LINE_CCD7_PIN
#define BSP_ADC_CCD_COUNT 8u
#else
#define BSP_ADC_CCD_COUNT 7u
#endif

void     BSP_Adc_Init(void);

/* 原始 12-bit 值 */
uint16_t BSP_Adc_GetBattery(void);

/* 返回电池电压（mV），已考虑分压比 */
uint16_t BSP_Adc_GetBatteryMv(void);

/* 7/8 路灰度，v[0..BSP_ADC_CCD_COUNT-1] */
void     BSP_Adc_GetCCD(uint16_t v[BSP_ADC_CCD_COUNT]);

DevLineSensor *BSP_Adc_GetLineSensor(void);

#endif
