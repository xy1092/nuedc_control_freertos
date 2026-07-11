#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "ti_msp_dl_config.h"

/*
 * Logical signal aliases over SysConfig-generated names.
 * When the board changes, update this file first. Upper layers should not
 * include ti_msp_dl_config.h directly.
 */

/* Motor PWM: TIMA0 C0/C1 on the reference board. */
#define PWM_CH_LEFT             GPIO_PWM_MOTOR_C0_IDX
#define PWM_CH_RIGHT            GPIO_PWM_MOTOR_C1_IDX

/* H-bridge direction pins. */
#define MOTOR_DIR_PORT          GPIO_MOTOR_DIR_PORT
#define LEFT_AIN1_PIN           GPIO_MOTOR_DIR_LEFT_AIN1_PIN
#define LEFT_AIN2_PIN           GPIO_MOTOR_DIR_LEFT_AIN2_PIN
#define RIGHT_BIN1_PIN          GPIO_MOTOR_DIR_RIGHT_BIN1_PIN
#define RIGHT_BIN2_PIN          GPIO_MOTOR_DIR_RIGHT_BIN2_PIN
#ifdef GPIO_MOTOR_DIR_STBY_PIN
#define MOTOR_STBY_PIN          GPIO_MOTOR_DIR_STBY_PIN
#endif

/* Quadrature encoder pins. */
#define ENC_LEFT_PORT           GPIO_ENCODER_A_PORT
#define ENC_LEFT_A_PIN          GPIO_ENCODER_A_LEFT_A_PIN
#define ENC_LEFT_B_PIN          GPIO_ENCODER_A_LEFT_B_PIN
#define ENC_RIGHT_PORT          GPIO_ENCODER_B_PORT
#define ENC_RIGHT_A_PIN         GPIO_ENCODER_B_RIGHT_A_PIN
#define ENC_RIGHT_B_PIN         GPIO_ENCODER_B_RIGHT_B_PIN

/* ICM45688 I2C bus and latched DRDY interrupt. */
#ifdef I2C_INST_INST
#define I2C_INST                I2C_INST_INST
#endif
#define ICM45688_I2C_ADDR       0x68u
#define IMU_I2C_ADDR            ICM45688_I2C_ADDR
#define IMU_INT_PORT            GPIO_IMU_PORT
#define IMU_INT_PIN             GPIO_IMU_ICM_INT_PIN

/* Optional K230/OpenMV vision UART. Add UART_K230 in SysConfig before enabling. */
#ifdef UART_K230_INST
#define K230_UART_INST          UART_K230_INST
#define K230_UART_IRQN          UART_K230_INST_INT_IRQN
#elif defined(NUEDC_USE_K230)
#error "NUEDC_USE_K230 requires UART_K230 in SysConfig."
#endif

/* 7/8-channel digital line sensor, logical order: front-left to front-right. */
#define LINE_A_PORT             GPIO_LINE_A_PORT
#define LINE_CCD0_PIN           GPIO_LINE_A_CCD0_PIN
#define LINE_CCD1_PIN           GPIO_LINE_A_CCD1_PIN
#define LINE_CCD2_PIN           GPIO_LINE_A_CCD2_PIN
#define LINE_CCD3_PIN           GPIO_LINE_A_CCD3_PIN
#define LINE_CCD4_PIN           GPIO_LINE_A_CCD4_PIN
#define LINE_B_PORT             GPIO_LINE_B_PORT
#define LINE_CCD5_PIN           GPIO_LINE_B_CCD5_PIN
#define LINE_CCD6_PIN           GPIO_LINE_B_CCD6_PIN
#ifdef GPIO_LINE_B_CCD7_PIN
#define LINE_CCD7_PIN           GPIO_LINE_B_CCD7_PIN
#endif

#endif /* PIN_MAP_H */
