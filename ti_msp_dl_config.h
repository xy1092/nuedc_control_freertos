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




/* Defines for I2C_INST */
#define I2C_INST_INST                                                       I2C1
#define I2C_INST_INST_IRQHandler                                 I2C1_IRQHandler
#define I2C_INST_INST_INT_IRQN                                     I2C1_INT_IRQn
#define I2C_INST_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_INST_SDA_PORT                                             GPIOA
#define GPIO_I2C_INST_SDA_PIN                                     DL_GPIO_PIN_16
#define GPIO_I2C_INST_IOMUX_SDA                                  (IOMUX_PINCM38)
#define GPIO_I2C_INST_IOMUX_SDA_FUNC                   IOMUX_PINCM38_PF_I2C1_SDA
#define GPIO_I2C_INST_SCL_PORT                                             GPIOA
#define GPIO_I2C_INST_SCL_PIN                                     DL_GPIO_PIN_17
#define GPIO_I2C_INST_IOMUX_SCL                                  (IOMUX_PINCM39)
#define GPIO_I2C_INST_IOMUX_SCL_FUNC                   IOMUX_PINCM39_PF_I2C1_SCL


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





/* Port definition for Pin Group GPIO_NOTIFY */
#define GPIO_NOTIFY_PORT                                                 (GPIOA)

/* Defines for LED: GPIOA.14 with pinCMx 36 on package pin 29 */
#define GPIO_NOTIFY_LED_PIN                                     (DL_GPIO_PIN_14)
#define GPIO_NOTIFY_LED_IOMUX                                    (IOMUX_PINCM36)
/* Port definition for Pin Group GPIO_BUZZER */
#define GPIO_BUZZER_PORT                                                 (GPIOB)

/* Defines for BUZZ: GPIOB.7 with pinCMx 24 on package pin 21 */
#define GPIO_BUZZER_BUZZ_PIN                                     (DL_GPIO_PIN_7)
#define GPIO_BUZZER_BUZZ_IOMUX                                   (IOMUX_PINCM24)
/* Port definition for Pin Group GPIO_IMU */
#define GPIO_IMU_PORT                                                    (GPIOA)

/* Defines for ICM_INT: GPIOA.15 with pinCMx 37 on package pin 30 */
#define GPIO_IMU_ICM_INT_PIN                                    (DL_GPIO_PIN_15)
#define GPIO_IMU_ICM_INT_IOMUX                                   (IOMUX_PINCM37)
/* Port definition for Pin Group GPIO_CONTROL */
#define GPIO_CONTROL_PORT                                                (GPIOA)

/* Defines for START: GPIOA.18 with pinCMx 40 on package pin 33 */
#define GPIO_CONTROL_START_PIN                                  (DL_GPIO_PIN_18)
#define GPIO_CONTROL_START_IOMUX                                 (IOMUX_PINCM40)
/* Defines for MODE0: GPIOA.21 with pinCMx 46 on package pin 39 */
#define GPIO_CONTROL_MODE0_PIN                                  (DL_GPIO_PIN_21)
#define GPIO_CONTROL_MODE0_IOMUX                                 (IOMUX_PINCM46)
/* Defines for MODE1: GPIOA.22 with pinCMx 47 on package pin 40 */
#define GPIO_CONTROL_MODE1_PIN                                  (DL_GPIO_PIN_22)
#define GPIO_CONTROL_MODE1_IOMUX                                 (IOMUX_PINCM47)
/* Defines for STOP: GPIOA.28 with pinCMx 3 on package pin 3 */
#define GPIO_CONTROL_STOP_PIN                                   (DL_GPIO_PIN_28)
#define GPIO_CONTROL_STOP_IOMUX                                   (IOMUX_PINCM3)
/* Port definition for Pin Group GPIO_MOTOR_DIR */
#define GPIO_MOTOR_DIR_PORT                                              (GPIOB)

/* Defines for LEFT_AIN1: GPIOB.2 with pinCMx 15 on package pin 14 */
#define GPIO_MOTOR_DIR_LEFT_AIN1_PIN                             (DL_GPIO_PIN_2)
#define GPIO_MOTOR_DIR_LEFT_AIN1_IOMUX                           (IOMUX_PINCM15)
/* Defines for LEFT_AIN2: GPIOB.3 with pinCMx 16 on package pin 15 */
#define GPIO_MOTOR_DIR_LEFT_AIN2_PIN                             (DL_GPIO_PIN_3)
#define GPIO_MOTOR_DIR_LEFT_AIN2_IOMUX                           (IOMUX_PINCM16)
/* Defines for RIGHT_BIN1: GPIOB.8 with pinCMx 25 on package pin 22 */
#define GPIO_MOTOR_DIR_RIGHT_BIN1_PIN                            (DL_GPIO_PIN_8)
#define GPIO_MOTOR_DIR_RIGHT_BIN1_IOMUX                          (IOMUX_PINCM25)
/* Defines for RIGHT_BIN2: GPIOB.9 with pinCMx 26 on package pin 23 */
#define GPIO_MOTOR_DIR_RIGHT_BIN2_PIN                            (DL_GPIO_PIN_9)
#define GPIO_MOTOR_DIR_RIGHT_BIN2_IOMUX                          (IOMUX_PINCM26)
/* Defines for STBY: GPIOB.6 with pinCMx 23 on package pin 20 */
#define GPIO_MOTOR_DIR_STBY_PIN                                  (DL_GPIO_PIN_6)
#define GPIO_MOTOR_DIR_STBY_IOMUX                                (IOMUX_PINCM23)
/* Port definition for Pin Group GPIO_RESERVED */
#define GPIO_RESERVED_PORT                                               (GPIOA)

/* Defines for ULTRASONIC_TRIG: GPIOA.12 with pinCMx 34 on package pin 27 */
#define GPIO_RESERVED_ULTRASONIC_TRIG_PIN                       (DL_GPIO_PIN_12)
#define GPIO_RESERVED_ULTRASONIC_TRIG_IOMUX                      (IOMUX_PINCM34)
/* Defines for ULTRASONIC_ECHO: GPIOA.13 with pinCMx 35 on package pin 28 */
#define GPIO_RESERVED_ULTRASONIC_ECHO_PIN                       (DL_GPIO_PIN_13)
#define GPIO_RESERVED_ULTRASONIC_ECHO_IOMUX                      (IOMUX_PINCM35)
/* Port definition for Pin Group GPIO_LINE_A */
#define GPIO_LINE_A_PORT                                                 (GPIOA)

/* Defines for CCD0: GPIOA.27 with pinCMx 60 on package pin 47 */
#define GPIO_LINE_A_CCD0_PIN                                    (DL_GPIO_PIN_27)
#define GPIO_LINE_A_CCD0_IOMUX                                   (IOMUX_PINCM60)
/* Defines for CCD1: GPIOA.26 with pinCMx 59 on package pin 46 */
#define GPIO_LINE_A_CCD1_PIN                                    (DL_GPIO_PIN_26)
#define GPIO_LINE_A_CCD1_IOMUX                                   (IOMUX_PINCM59)
/* Defines for CCD2: GPIOA.25 with pinCMx 55 on package pin 45 */
#define GPIO_LINE_A_CCD2_PIN                                    (DL_GPIO_PIN_25)
#define GPIO_LINE_A_CCD2_IOMUX                                   (IOMUX_PINCM55)
/* Defines for CCD3: GPIOA.24 with pinCMx 54 on package pin 44 */
#define GPIO_LINE_A_CCD3_PIN                                    (DL_GPIO_PIN_24)
#define GPIO_LINE_A_CCD3_IOMUX                                   (IOMUX_PINCM54)
/* Defines for CCD4: GPIOA.7 with pinCMx 14 on package pin 13 */
#define GPIO_LINE_A_CCD4_PIN                                     (DL_GPIO_PIN_7)
#define GPIO_LINE_A_CCD4_IOMUX                                   (IOMUX_PINCM14)
/* Port definition for Pin Group GPIO_LINE_B */
#define GPIO_LINE_B_PORT                                                 (GPIOB)

/* Defines for CCD5: GPIOB.18 with pinCMx 44 on package pin 37 */
#define GPIO_LINE_B_CCD5_PIN                                    (DL_GPIO_PIN_18)
#define GPIO_LINE_B_CCD5_IOMUX                                   (IOMUX_PINCM44)
/* Defines for CCD6: GPIOB.19 with pinCMx 45 on package pin 38 */
#define GPIO_LINE_B_CCD6_PIN                                    (DL_GPIO_PIN_19)
#define GPIO_LINE_B_CCD6_IOMUX                                   (IOMUX_PINCM45)
/* Port definition for Pin Group GPIO_ENCODER_A */
#define GPIO_ENCODER_A_PORT                                              (GPIOA)

/* Defines for LEFT_A: GPIOA.0 with pinCMx 1 on package pin 1 */
#define GPIO_ENCODER_A_LEFT_A_PIN                                (DL_GPIO_PIN_0)
#define GPIO_ENCODER_A_LEFT_A_IOMUX                               (IOMUX_PINCM1)
/* Defines for LEFT_B: GPIOA.1 with pinCMx 2 on package pin 2 */
#define GPIO_ENCODER_A_LEFT_B_PIN                                (DL_GPIO_PIN_1)
#define GPIO_ENCODER_A_LEFT_B_IOMUX                               (IOMUX_PINCM2)
/* Port definition for Pin Group GPIO_ENCODER_B */
#define GPIO_ENCODER_B_PORT                                              (GPIOB)

/* Defines for RIGHT_A: GPIOB.20 with pinCMx 48 on package pin 41 */
#define GPIO_ENCODER_B_RIGHT_A_PIN                              (DL_GPIO_PIN_20)
#define GPIO_ENCODER_B_RIGHT_A_IOMUX                             (IOMUX_PINCM48)
/* Defines for RIGHT_B: GPIOB.24 with pinCMx 52 on package pin 42 */
#define GPIO_ENCODER_B_RIGHT_B_PIN                              (DL_GPIO_PIN_24)
#define GPIO_ENCODER_B_RIGHT_B_IOMUX                             (IOMUX_PINCM52)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_SYSCTL_CLK_init(void);
void SYSCFG_DL_PWM_MOTOR_init(void);
void SYSCFG_DL_I2C_INST_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
