#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "ti_msp_dl_config.h"

/* Logical aliases over SysConfig names. Physical board changes stop here. */

/* Motor PWM and H-bridge. STBY is tied directly to 3V3. */
#define PWM_CH_LEFT             GPIO_PWM_MOTOR_C0_IDX
#define PWM_CH_RIGHT            GPIO_PWM_MOTOR_C1_IDX
#define MOTOR_A_DIR_PORT        GPIO_MOTOR_A_PORT
#define LEFT_AIN1_PIN           GPIO_MOTOR_A_LEFT_AIN1_PIN
#define LEFT_AIN2_PIN           GPIO_MOTOR_A_LEFT_AIN2_PIN
#define RIGHT_BIN1_PIN          GPIO_MOTOR_A_RIGHT_BIN1_PIN
#define MOTOR_B_DIR_PORT        GPIO_MOTOR_B_PORT
#define RIGHT_BIN2_PIN          GPIO_MOTOR_B_RIGHT_BIN2_PIN

/* Quadrature encoders: native QEI on TIMG8, dual-edge capture on TIMG6. */
#define ENC_LEFT_TIMER          QEI_LEFT_INST
#define ENC_LEFT_TIMER_IRQN     QEI_LEFT_INST_INT_IRQN
#define ENC_LEFT_PORT           GPIO_QEI_LEFT_PHA_PORT
#define ENC_LEFT_A_PIN          GPIO_QEI_LEFT_PHA_PIN
#define ENC_LEFT_B_PIN          GPIO_QEI_LEFT_PHB_PIN
#define ENC_RIGHT_TIMER         CAPTURE_RIGHT_INST
#define ENC_RIGHT_TIMER_IRQN    CAPTURE_RIGHT_INST_INT_IRQN
#define ENC_RIGHT_PORT          GPIO_CAPTURE_RIGHT_C0_PORT
#define ENC_RIGHT_A_PIN         GPIO_CAPTURE_RIGHT_C0_PIN
#define ENC_RIGHT_B_PIN         GPIO_CAPTURE_RIGHT_C1_PIN

/* ICM45688 SPI1 mode 3 at 2 MHz, manual CS and latched DRDY. */
#define IMU_SPI_INST            SPI_ICM45688_INST
#define IMU_CS_PORT             GPIO_IMU_CS_PORT
#define IMU_CS_PIN              GPIO_IMU_CS_ICM_CS_PIN
#define IMU_INT_PORT            GPIO_IMU_INT_PORT
#define IMU_INT_PIN             GPIO_IMU_INT_ICM_INT1_PIN

/* OLED I2C0 at 400 kHz. */
#define OLED_I2C_INST           I2C_OLED_INST

/* User inputs and indicators. */
#define CALIBRATE_BUTTON_PORT   GPIO_BUTTON_A_PORT
#define CALIBRATE_BUTTON_PIN    GPIO_BUTTON_A_CALIBRATE_PIN
#define MODE_BUTTON_PORT        GPIO_BUTTON_B_PORT
#define MODE_BUTTON_PIN         GPIO_BUTTON_B_MODE_PIN
#define START_STOP_BUTTON_PORT  GPIO_BUTTON_B_PORT
#define START_STOP_BUTTON_PIN   GPIO_BUTTON_B_START_STOP_PIN
#define BUZZER_PORT             GPIO_BUZZER_PORT
#define BUZZER_PIN              GPIO_BUZZER_BUZZ_PIN
#define STATUS_LED_PORT         GPIO_LED_PORT
#define STATUS_LED_PIN          GPIO_LED_LED_PIN

/* Optional K230/OpenMV vision UART. Add UART_K230 in SysConfig first. */
#ifdef UART_K230_INST
#define K230_UART_INST          UART_K230_INST
#define K230_UART_IRQN          UART_K230_INST_INT_IRQN
#elif defined(NUEDC_USE_K230)
#error "NUEDC_USE_K230 requires UART_K230 in SysConfig."
#endif

/* Seven digital grayscale channels, physical order: left to right. */
#define LINE_CCD0_PORT          GPIO_LINE_B_PORT
#define LINE_CCD0_PIN           GPIO_LINE_B_LINE6_PIN
#define LINE_CCD1_PORT          GPIO_LINE_B_PORT
#define LINE_CCD1_PIN           GPIO_LINE_B_LINE5_PIN
#define LINE_CCD2_PORT          GPIO_LINE_B_PORT
#define LINE_CCD2_PIN           GPIO_LINE_B_LINE4_PIN
#define LINE_CCD3_PORT          GPIO_LINE_A_PORT
#define LINE_CCD3_PIN           GPIO_LINE_A_LINE3_PIN
#define LINE_CCD4_PORT          GPIO_LINE_A_PORT
#define LINE_CCD4_PIN           GPIO_LINE_A_LINE2_PIN
#define LINE_CCD5_PORT          GPIO_LINE_A_PORT
#define LINE_CCD5_PIN           GPIO_LINE_A_LINE1_PIN
#define LINE_CCD6_PORT          GPIO_LINE_A_PORT
#define LINE_CCD6_PIN           GPIO_LINE_A_LINE0_PIN

#endif
