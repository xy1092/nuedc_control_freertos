/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gPWM_MOTORBackup;
DL_TimerG_backupConfig gQEI_LEFTBackup;
DL_TimerG_backupConfig gCAPTURE_RIGHTBackup;
DL_SPI_backupConfig gSPI_ICM45688Backup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_MOTOR_init();
    SYSCFG_DL_QEI_LEFT_init();
    SYSCFG_DL_CAPTURE_RIGHT_init();
    SYSCFG_DL_I2C_OLED_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_SPI_ICM45688_init();
    SYSCFG_DL_WWDT0_init();
    SYSCFG_DL_SYSCTL_CLK_init();
    /* Ensure backup structures have no valid state */
	gPWM_MOTORBackup.backupRdy 	= false;
	gQEI_LEFTBackup.backupRdy 	= false;
	gCAPTURE_RIGHTBackup.backupRdy 	= false;

	gSPI_ICM45688Backup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(PWM_MOTOR_INST, &gPWM_MOTORBackup);
	retStatus &= DL_TimerG_saveConfiguration(QEI_LEFT_INST, &gQEI_LEFTBackup);
	retStatus &= DL_TimerG_saveConfiguration(CAPTURE_RIGHT_INST, &gCAPTURE_RIGHTBackup);
	retStatus &= DL_SPI_saveConfiguration(SPI_ICM45688_INST, &gSPI_ICM45688Backup);

    return retStatus;
}
SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(PWM_MOTOR_INST, &gPWM_MOTORBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(QEI_LEFT_INST, &gQEI_LEFTBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(CAPTURE_RIGHT_INST, &gCAPTURE_RIGHTBackup, false);
	retStatus &= DL_SPI_restoreConfiguration(SPI_ICM45688_INST, &gSPI_ICM45688Backup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(PWM_MOTOR_INST);
    DL_TimerG_reset(QEI_LEFT_INST);
    DL_TimerG_reset(CAPTURE_RIGHT_INST);
    DL_I2C_reset(I2C_OLED_INST);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_SPI_reset(SPI_ICM45688_INST);
    DL_WWDT_reset(WWDT0_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(PWM_MOTOR_INST);
    DL_TimerG_enablePower(QEI_LEFT_INST);
    DL_TimerG_enablePower(CAPTURE_RIGHT_INST);
    DL_I2C_enablePower(I2C_OLED_INST);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_SPI_enablePower(SPI_ICM45688_INST);
    DL_WWDT_enablePower(WWDT0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_MOTOR_C0_IOMUX,GPIO_PWM_MOTOR_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_MOTOR_C0_PORT, GPIO_PWM_MOTOR_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_MOTOR_C1_IOMUX,GPIO_PWM_MOTOR_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_MOTOR_C1_PORT, GPIO_PWM_MOTOR_C1_PIN);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_QEI_LEFT_PHA_IOMUX,
        GPIO_QEI_LEFT_PHA_IOMUX_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_QEI_LEFT_PHB_IOMUX,
        GPIO_QEI_LEFT_PHB_IOMUX_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_CAPTURE_RIGHT_C0_IOMUX,
        GPIO_CAPTURE_RIGHT_C0_IOMUX_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_CAPTURE_RIGHT_C1_IOMUX,
        GPIO_CAPTURE_RIGHT_C1_IOMUX_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_PULL_UP, DL_GPIO_HYSTERESIS_ENABLE,
        DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_OLED_IOMUX_SDA,
        GPIO_I2C_OLED_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_OLED_IOMUX_SCL,
        GPIO_I2C_OLED_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_OLED_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_OLED_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_ICM45688_IOMUX_SCLK, GPIO_SPI_ICM45688_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_ICM45688_IOMUX_PICO, GPIO_SPI_ICM45688_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_ICM45688_IOMUX_POCI, GPIO_SPI_ICM45688_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(GPIO_LED_LED_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_BUTTON_A_CALIBRATE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_B_RIGHT_BIN2_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BUZZER_BUZZ_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_IMU_CS_ICM_CS_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_IMU_INT_ICM_INT1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_BUTTON_B_MODE_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_BUTTON_B_START_STOP_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_A_LEFT_AIN1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_A_LEFT_AIN2_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_MOTOR_A_RIGHT_BIN1_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_A_LINE0_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_A_LINE1_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_A_LINE2_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_A_LINE3_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_B_LINE4_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_B_LINE5_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_LINE_B_LINE6_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_clearPins(GPIOA, GPIO_MOTOR_A_LEFT_AIN1_PIN |
		GPIO_MOTOR_A_LEFT_AIN2_PIN |
		GPIO_MOTOR_A_RIGHT_BIN1_PIN);
    DL_GPIO_enableOutput(GPIOA, GPIO_MOTOR_A_LEFT_AIN1_PIN |
		GPIO_MOTOR_A_LEFT_AIN2_PIN |
		GPIO_MOTOR_A_RIGHT_BIN1_PIN);
    DL_GPIO_clearPins(GPIOB, GPIO_LED_LED_PIN |
		GPIO_MOTOR_B_RIGHT_BIN2_PIN |
		GPIO_BUZZER_BUZZ_PIN);
    DL_GPIO_setPins(GPIOB, GPIO_IMU_CS_ICM_CS_PIN);
    DL_GPIO_enableOutput(GPIOB, GPIO_LED_LED_PIN |
		GPIO_MOTOR_B_RIGHT_BIN2_PIN |
		GPIO_BUZZER_BUZZ_PIN |
		GPIO_IMU_CS_ICM_CS_PIN);

}



SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();

}
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_CLK_init(void) {
    while ((DL_SYSCTL_getClockStatus() & (DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	       != (DL_SYSCTL_CLK_STATUS_LFOSC_GOOD))
	{
		/* Ensure that clocks are in default POR configuration before initialization.
		* Additionally once LFXT is enabled, the internal LFOSC is disabled, and cannot
		* be re-enabled other than by executing a BOOTRST. */
		;
	}
}



/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gPWM_MOTORClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gPWM_MOTORConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1000,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_START,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_MOTOR_init(void) {

    DL_TimerA_setClockConfig(
        PWM_MOTOR_INST, (DL_TimerA_ClockConfig *) &gPWM_MOTORClockConfig);

    DL_TimerA_initPWMMode(
        PWM_MOTOR_INST, (DL_TimerA_PWMConfig *) &gPWM_MOTORConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(PWM_MOTOR_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(PWM_MOTOR_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_MOTOR_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, 1000, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(PWM_MOTOR_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(PWM_MOTOR_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, 1000, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(PWM_MOTOR_INST);


    
    DL_TimerA_setCCPDirection(PWM_MOTOR_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}


static const DL_TimerG_ClockConfig gQEI_LEFTClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};


SYSCONFIG_WEAK void SYSCFG_DL_QEI_LEFT_init(void) {

    DL_TimerG_setClockConfig(
        QEI_LEFT_INST, (DL_TimerG_ClockConfig *) &gQEI_LEFTClockConfig);

    DL_TimerG_configQEI(QEI_LEFT_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_0_INDEX);
    DL_TimerG_configQEI(QEI_LEFT_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setLoadValue(QEI_LEFT_INST, 65535);
    DL_TimerG_enableClock(QEI_LEFT_INST);
}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerG_ClockConfig gCAPTURE_RIGHTClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * CAPTURE_RIGHT_INST_LOAD_VALUE = (1 ms * 32000000 Hz) - 1
 */

SYSCONFIG_WEAK void SYSCFG_DL_CAPTURE_RIGHT_init(void) {

    DL_TimerG_setClockConfig(CAPTURE_RIGHT_INST,
        (DL_TimerG_ClockConfig *) &gCAPTURE_RIGHTClockConfig);

    DL_TimerG_setLoadValue(CAPTURE_RIGHT_INST,31999);

    DL_TimerG_setCounterMode(CAPTURE_RIGHT_INST,DL_TIMER_COUNT_MODE_UP);

    DL_TimerG_setCounterRepeatMode(CAPTURE_RIGHT_INST,DL_TIMER_REPEAT_MODE_ENABLED);

    DL_TimerG_setCounterValueAfterEnable(CAPTURE_RIGHT_INST,DL_TIMER_COUNT_AFTER_EN_ZERO);

    DL_TimerG_setCaptureCompareCtl(CAPTURE_RIGHT_INST,
    DL_TIMER_CC_MODE_CAPTURE, (DL_TIMER_CC_ZCOND_TRIG_FALL | DL_TIMER_CC_ACOND_TIMCLK | DL_TIMER_CC_CCOND_TRIG_EDGE),
    DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareInput(CAPTURE_RIGHT_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT,DL_TIMER_CC_IN_SEL_CCPX, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareCtl(CAPTURE_RIGHT_INST,
    DL_TIMER_CC_MODE_CAPTURE, (DL_TIMER_CC_ZCOND_NONE | DL_TIMER_CC_ACOND_TIMCLK | DL_TIMER_CC_CCOND_TRIG_EDGE),
    DL_TIMER_CC_1_INDEX);

    DL_TimerG_setCaptureCompareInput(CAPTURE_RIGHT_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT,DL_TIMER_CC_IN_SEL_CCPX, DL_TIMER_CC_1_INDEX);


    DL_TimerG_setCounterControl(CAPTURE_RIGHT_INST,
        DL_TIMER_CZC_CCCTL0_ZCOND,
        DL_TIMER_CAC_CCCTL0_ACOND,
        DL_TIMER_CLC_CCCTL0_LCOND
    );

    DL_TimerG_startCounter(CAPTURE_RIGHT_INST);

    DL_TimerG_enableInterrupt(CAPTURE_RIGHT_INST , DL_TIMERG_INTERRUPT_CC0_DN_EVENT |
		DL_TIMERG_INTERRUPT_CC0_UP_EVENT |
		DL_TIMERG_INTERRUPT_CC1_DN_EVENT |
		DL_TIMERG_INTERRUPT_CC1_UP_EVENT);

    DL_TimerG_enableClock(CAPTURE_RIGHT_INST);

}

static const DL_I2C_ClockConfig gI2C_OLEDClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_OLED_init(void) {

    DL_I2C_setClockConfig(I2C_OLED_INST,
        (DL_I2C_ClockConfig *) &gI2C_OLEDClockConfig);
    DL_I2C_disableAnalogGlitchFilter(I2C_OLED_INST);

    /* Configure Controller Mode */
    DL_I2C_resetControllerTransfer(I2C_OLED_INST);
    /* Set frequency to 400000 Hz*/
    DL_I2C_setTimerPeriod(I2C_OLED_INST, 7);
    DL_I2C_setControllerTXFIFOThreshold(I2C_OLED_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C_OLED_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C_OLED_INST);


    /* Enable module */
    DL_I2C_enableController(I2C_OLED_INST);


}

static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DEBUG_init(void)
{
    DL_UART_Main_setClockConfig(UART_DEBUG_INST, (DL_UART_Main_ClockConfig *) &gUART_DEBUGClockConfig);

    DL_UART_Main_init(UART_DEBUG_INST, (DL_UART_Main_Config *) &gUART_DEBUGConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(UART_DEBUG_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_DEBUG_INST, UART_DEBUG_IBRD_32_MHZ_115200_BAUD, UART_DEBUG_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(UART_DEBUG_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);

    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(UART_DEBUG_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_DEBUG_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(UART_DEBUG_INST, DL_UART_TX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_enable(UART_DEBUG_INST);
}

static const DL_SPI_Config gSPI_ICM45688_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gSPI_ICM45688_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_ICM45688_init(void) {
    DL_SPI_setClockConfig(SPI_ICM45688_INST, (DL_SPI_ClockConfig *) &gSPI_ICM45688_clockConfig);

    DL_SPI_init(SPI_ICM45688_INST, (DL_SPI_Config *) &gSPI_ICM45688_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     2000000 = (32000000)/((1 + 7) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(SPI_ICM45688_INST, 7);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(SPI_ICM45688_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(SPI_ICM45688_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_WWDT0_init(void)
{
    /*
     * Initialize WWDT0 in Watchdog mode with following settings
     *   Watchdog Source Clock = (LFCLK Freq) / (WWDT Clock Divider)
     *                         = 32768Hz / 4 = 8.19 kHz
     *   Watchdog Period       = (WWDT Clock Divider) ∗ (WWDT Period Count) / 32768Hz
     *                         = 4 * 2^12 / 32768Hz = 500.00 ms
     *   Window0 Closed Period = (WWDT Period) * (Window0 Closed Percent)
     *                         = 500.00 ms * 0% = 0.00 s
     *   Window1 Closed Period = (WWDT Period) * (Window1 Closed Percent)
     *                         = 500.00 ms * 0% = 0.00 s
     */
    DL_WWDT_initWatchdogMode(WWDT0_INST, DL_WWDT_CLOCK_DIVIDE_4,
        DL_WWDT_TIMER_PERIOD_12_BITS, DL_WWDT_RUN_IN_SLEEP,
        DL_WWDT_WINDOW_PERIOD_0, DL_WWDT_WINDOW_PERIOD_0);

    /* Set Window0 as active window */
    DL_WWDT_setActiveWindow(WWDT0_INST, DL_WWDT_WINDOW0);

}
