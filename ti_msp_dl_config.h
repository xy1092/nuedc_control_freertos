/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_MOTOR */
#define PWM_MOTOR_INST                                                     TIMA0
#define PWM_MOTOR_INST_IRQHandler                               TIMA0_IRQHandler
#define PWM_MOTOR_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define PWM_MOTOR_INST_CLK_FREQ                                         32000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_MOTOR_C0_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C0_PIN                                      DL_GPIO_PIN_8
#define GPIO_PWM_MOTOR_C0_IOMUX                                  (IOMUX_PINCM19)
#define GPIO_PWM_MOTOR_C0_IOMUX_FUNC                 IOMUX_PINCM19_PF_TIMA0_CCP0
#define GPIO_PWM_MOTOR_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_MOTOR_C1_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C1_PIN                                      DL_GPIO_PIN_9
#define GPIO_PWM_MOTOR_C1_IOMUX                                  (IOMUX_PINCM20)
#define GPIO_PWM_MOTOR_C1_IOMUX_FUNC                 IOMUX_PINCM20_PF_TIMA0_CCP1
#define GPIO_PWM_MOTOR_C1_IDX                                DL_TIMER_CC_1_INDEX




/* Defines for QEI_LEFT */
#define QEI_LEFT_INST                                                      TIMG8
#define QEI_LEFT_INST_IRQHandler                                TIMG8_IRQHandler
#define QEI_LEFT_INST_INT_IRQN                                  (TIMG8_INT_IRQn)
/* Pin configuration defines for QEI_LEFT PHA Pin */
#define GPIO_QEI_LEFT_PHA_PORT                                             GPIOB
#define GPIO_QEI_LEFT_PHA_PIN                                     DL_GPIO_PIN_15
#define GPIO_QEI_LEFT_PHA_IOMUX                                  (IOMUX_PINCM32)
#define GPIO_QEI_LEFT_PHA_IOMUX_FUNC                 IOMUX_PINCM32_PF_TIMG8_CCP0
/* Pin configuration defines for QEI_LEFT PHB Pin */
#define GPIO_QEI_LEFT_PHB_PORT                                             GPIOB
#define GPIO_QEI_LEFT_PHB_PIN                                     DL_GPIO_PIN_16
#define GPIO_QEI_LEFT_PHB_IOMUX                                  (IOMUX_PINCM33)
#define GPIO_QEI_LEFT_PHB_IOMUX_FUNC                 IOMUX_PINCM33_PF_TIMG8_CCP1


/* Defines for CAPTURE_RIGHT */
#define CAPTURE_RIGHT_INST                                               (TIMG6)
#define CAPTURE_RIGHT_INST_IRQHandler                           TIMG6_IRQHandler
#define CAPTURE_RIGHT_INST_INT_IRQN                             (TIMG6_INT_IRQn)
#define CAPTURE_RIGHT_INST_LOAD_VALUE                                   (31999U)
/* GPIO defines for channel 0 */
#define GPIO_CAPTURE_RIGHT_C0_PORT                                         GPIOB
#define GPIO_CAPTURE_RIGHT_C0_PIN                                 DL_GPIO_PIN_26
#define GPIO_CAPTURE_RIGHT_C0_IOMUX                              (IOMUX_PINCM57)
#define GPIO_CAPTURE_RIGHT_C0_IOMUX_FUNC             IOMUX_PINCM57_PF_TIMG6_CCP0
/* GPIO defines for channel 1 */
#define GPIO_CAPTURE_RIGHT_C1_PORT                                         GPIOB
#define GPIO_CAPTURE_RIGHT_C1_PIN                                 DL_GPIO_PIN_27
#define GPIO_CAPTURE_RIGHT_C1_IOMUX                              (IOMUX_PINCM58)
#define GPIO_CAPTURE_RIGHT_C1_IOMUX_FUNC             IOMUX_PINCM58_PF_TIMG6_CCP1






/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C0
#define I2C_OLED_INST_IRQHandler                                 I2C0_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C0_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOA
#define GPIO_I2C_OLED_SDA_PIN                                      DL_GPIO_PIN_0
#define GPIO_I2C_OLED_IOMUX_SDA                                   (IOMUX_PINCM1)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                    IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOA
#define GPIO_I2C_OLED_SCL_PIN                                      DL_GPIO_PIN_1
#define GPIO_I2C_OLED_IOMUX_SCL                                   (IOMUX_PINCM2)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                    IOMUX_PINCM2_PF_I2C0_SCL


/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_FREQUENCY                                       32000000
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                            (115200)
#define UART_DEBUG_IBRD_32_MHZ_115200_BAUD                                  (17)
#define UART_DEBUG_FBRD_32_MHZ_115200_BAUD                                  (23)




/* Defines for SPI_ICM45688 */
#define SPI_ICM45688_INST                                                  SPI1
#define SPI_ICM45688_INST_IRQHandler                            SPI1_IRQHandler
#define SPI_ICM45688_INST_INT_IRQN                                SPI1_INT_IRQn
#define GPIO_SPI_ICM45688_PICO_PORT                                       GPIOB
#define GPIO_SPI_ICM45688_PICO_PIN                                DL_GPIO_PIN_8
#define GPIO_SPI_ICM45688_IOMUX_PICO                            (IOMUX_PINCM25)
#define GPIO_SPI_ICM45688_IOMUX_PICO_FUNC            IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_ICM45688_POCI_PORT                                       GPIOB
#define GPIO_SPI_ICM45688_POCI_PIN                                DL_GPIO_PIN_7
#define GPIO_SPI_ICM45688_IOMUX_POCI                            (IOMUX_PINCM24)
#define GPIO_SPI_ICM45688_IOMUX_POCI_FUNC            IOMUX_PINCM24_PF_SPI1_POCI
/* GPIO configuration for SPI_ICM45688 */
#define GPIO_SPI_ICM45688_SCLK_PORT                                       GPIOB
#define GPIO_SPI_ICM45688_SCLK_PIN                                DL_GPIO_PIN_9
#define GPIO_SPI_ICM45688_IOMUX_SCLK                            (IOMUX_PINCM26)
#define GPIO_SPI_ICM45688_IOMUX_SCLK_FUNC            IOMUX_PINCM26_PF_SPI1_SCLK



/* Port definition for Pin Group GPIO_LED */
#define GPIO_LED_PORT                                                    (GPIOB)

/* Defines for LED: GPIOB.22 with pinCMx 50 on package pin 21 */
#define GPIO_LED_LED_PIN                                        (DL_GPIO_PIN_22)
#define GPIO_LED_LED_IOMUX                                       (IOMUX_PINCM50)
/* Port definition for Pin Group GPIO_BUTTON_A */
#define GPIO_BUTTON_A_PORT                                               (GPIOA)

/* Defines for CALIBRATE: GPIOA.7 with pinCMx 14 on package pin 49 */
#define GPIO_BUTTON_A_CALIBRATE_PIN                              (DL_GPIO_PIN_7)
#define GPIO_BUTTON_A_CALIBRATE_IOMUX                            (IOMUX_PINCM14)
/* Port definition for Pin Group GPIO_MOTOR_B */
#define GPIO_MOTOR_B_PORT                                                (GPIOB)

/* Defines for RIGHT_BIN2: GPIOB.23 with pinCMx 51 on package pin 22 */
#define GPIO_MOTOR_B_RIGHT_BIN2_PIN                             (DL_GPIO_PIN_23)
#define GPIO_MOTOR_B_RIGHT_BIN2_IOMUX                            (IOMUX_PINCM51)
/* Port definition for Pin Group GPIO_BUZZER */
#define GPIO_BUZZER_PORT                                                 (GPIOB)

/* Defines for BUZZ: GPIOB.14 with pinCMx 31 on package pin 2 */
#define GPIO_BUZZER_BUZZ_PIN                                    (DL_GPIO_PIN_14)
#define GPIO_BUZZER_BUZZ_IOMUX                                   (IOMUX_PINCM31)
/* Port definition for Pin Group GPIO_IMU_CS */
#define GPIO_IMU_CS_PORT                                                 (GPIOB)

/* Defines for ICM_CS: GPIOB.6 with pinCMx 23 on package pin 58 */
#define GPIO_IMU_CS_ICM_CS_PIN                                   (DL_GPIO_PIN_6)
#define GPIO_IMU_CS_ICM_CS_IOMUX                                 (IOMUX_PINCM23)
/* Port definition for Pin Group GPIO_IMU_INT */
#define GPIO_IMU_INT_PORT                                                (GPIOA)

/* Defines for ICM_INT1: GPIOA.13 with pinCMx 35 on package pin 6 */
#define GPIO_IMU_INT_ICM_INT1_PIN                               (DL_GPIO_PIN_13)
#define GPIO_IMU_INT_ICM_INT1_IOMUX                              (IOMUX_PINCM35)
/* Port definition for Pin Group GPIO_BUTTON_B */
#define GPIO_BUTTON_B_PORT                                               (GPIOB)

/* Defines for MODE: GPIOB.1 with pinCMx 13 on package pin 48 */
#define GPIO_BUTTON_B_MODE_PIN                                   (DL_GPIO_PIN_1)
#define GPIO_BUTTON_B_MODE_IOMUX                                 (IOMUX_PINCM13)
/* Defines for START_STOP: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_BUTTON_B_START_STOP_PIN                            (DL_GPIO_PIN_21)
#define GPIO_BUTTON_B_START_STOP_IOMUX                           (IOMUX_PINCM49)
/* Port definition for Pin Group GPIO_MOTOR_A */
#define GPIO_MOTOR_A_PORT                                                (GPIOA)

/* Defines for LEFT_AIN1: GPIOA.15 with pinCMx 37 on package pin 8 */
#define GPIO_MOTOR_A_LEFT_AIN1_PIN                              (DL_GPIO_PIN_15)
#define GPIO_MOTOR_A_LEFT_AIN1_IOMUX                             (IOMUX_PINCM37)
/* Defines for LEFT_AIN2: GPIOA.16 with pinCMx 38 on package pin 9 */
#define GPIO_MOTOR_A_LEFT_AIN2_PIN                              (DL_GPIO_PIN_16)
#define GPIO_MOTOR_A_LEFT_AIN2_IOMUX                             (IOMUX_PINCM38)
/* Defines for RIGHT_BIN1: GPIOA.12 with pinCMx 34 on package pin 5 */
#define GPIO_MOTOR_A_RIGHT_BIN1_PIN                             (DL_GPIO_PIN_12)
#define GPIO_MOTOR_A_RIGHT_BIN1_IOMUX                            (IOMUX_PINCM34)
/* Port definition for Pin Group GPIO_LINE_A */
#define GPIO_LINE_A_PORT                                                 (GPIOA)

/* Defines for LINE0: GPIOA.27 with pinCMx 60 on package pin 31 */
#define GPIO_LINE_A_LINE0_PIN                                   (DL_GPIO_PIN_27)
#define GPIO_LINE_A_LINE0_IOMUX                                  (IOMUX_PINCM60)
/* Defines for LINE1: GPIOA.26 with pinCMx 59 on package pin 30 */
#define GPIO_LINE_A_LINE1_PIN                                   (DL_GPIO_PIN_26)
#define GPIO_LINE_A_LINE1_IOMUX                                  (IOMUX_PINCM59)
/* Defines for LINE2: GPIOA.25 with pinCMx 55 on package pin 26 */
#define GPIO_LINE_A_LINE2_PIN                                   (DL_GPIO_PIN_25)
#define GPIO_LINE_A_LINE2_IOMUX                                  (IOMUX_PINCM55)
/* Defines for LINE3: GPIOA.24 with pinCMx 54 on package pin 25 */
#define GPIO_LINE_A_LINE3_PIN                                   (DL_GPIO_PIN_24)
#define GPIO_LINE_A_LINE3_IOMUX                                  (IOMUX_PINCM54)
/* Port definition for Pin Group GPIO_LINE_B */
#define GPIO_LINE_B_PORT                                                 (GPIOB)

/* Defines for LINE4: GPIOB.25 with pinCMx 56 on package pin 27 */
#define GPIO_LINE_B_LINE4_PIN                                   (DL_GPIO_PIN_25)
#define GPIO_LINE_B_LINE4_IOMUX                                  (IOMUX_PINCM56)
/* Defines for LINE5: GPIOB.24 with pinCMx 52 on package pin 23 */
#define GPIO_LINE_B_LINE5_PIN                                   (DL_GPIO_PIN_24)
#define GPIO_LINE_B_LINE5_IOMUX                                  (IOMUX_PINCM52)
/* Defines for LINE6: GPIOB.20 with pinCMx 48 on package pin 19 */
#define GPIO_LINE_B_LINE6_PIN                                   (DL_GPIO_PIN_20)
#define GPIO_LINE_B_LINE6_IOMUX                                  (IOMUX_PINCM48)


/* Defines for WWDT */
#define WWDT0_INST                                                       (WWDT0)
#define WWDT0_INT_IRQN                                          (WWDT0_INT_IRQn)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);
void SYSCFG_DL_PWM_MOTOR_init(void);
void SYSCFG_DL_QEI_LEFT_init(void);
void SYSCFG_DL_CAPTURE_RIGHT_init(void);
void SYSCFG_DL_I2C_OLED_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_SPI_ICM45688_init(void);

void SYSCFG_DL_WWDT0_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
