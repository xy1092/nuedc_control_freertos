#include "ti_msp_dl_config.h"
#include "bsp_adc.h"
#include "drv_uart.h"
#include "pin_map.h"
#include "rtt_log.h"
#include <stdio.h>
#include <stdint.h>

#define GRAY_MASK          0x7Fu
#define SAMPLE_PERIOD_MS   10u
#define REPORT_PERIOD_MS   100u
#define HEARTBEAT_MS       500u

static const DrvUartPort s_uart = {
    UART_DEBUG_INST,
    UART_DEBUG_INST_INT_IRQN,
};

static void write_text(const char *text, uint16_t len)
{
    Drv_Uart_WriteBlocking(&s_uart, (const uint8_t *)text, len);
    RTT_Log_Write(text, len);
}

static void write_string(const char *text)
{
    uint16_t len = 0u;

    while (text[len] != '\0') {
        len++;
    }
    write_text(text, len);
}

static uint8_t read_gray_bits(void)
{
    uint16_t raw[BSP_ADC_CCD_COUNT];
    uint8_t bits = 0u;

    BSP_Adc_GetCCD(raw);
    for (uint8_t i = 0u; i < BSP_ADC_CCD_COUNT; ++i) {
        if (raw[i] != 0u) {
            bits |= (uint8_t)(1u << i);
        }
    }
    return bits;
}

static void report(uint32_t now_ms, uint8_t bits, uint8_t changed,
                   uint8_t seen_low, uint8_t seen_high,
                   const uint16_t edges[BSP_ADC_CCD_COUNT])
{
    char line[256];
    int len = snprintf(line, sizeof(line),
        "$GRAY,%lu,%u,%u,%u,%u,%u,%u,%u,0x%02X,0x%02X,0x%02X,0x%02X,"
        "%u,%u,%u,%u,%u,%u,%u\r\n",
        (unsigned long)now_ms,
        (bits >> 0) & 1u, (bits >> 1) & 1u, (bits >> 2) & 1u,
        (bits >> 3) & 1u, (bits >> 4) & 1u, (bits >> 5) & 1u,
        (bits >> 6) & 1u, bits, changed, seen_low, seen_high,
        edges[0], edges[1], edges[2], edges[3],
        edges[4], edges[5], edges[6]);

    if (len > 0) {
        uint16_t count = (len < (int)sizeof(line)) ? (uint16_t)len :
                                                     (uint16_t)(sizeof(line) - 1u);
        write_text(line, count);
    }
}

int main(void)
{
    static uint16_t edges[BSP_ADC_CCD_COUNT];
    uint32_t now_ms = 0u;
    uint32_t report_elapsed = 0u;
    uint32_t heartbeat_elapsed = 0u;
    uint8_t previous;
    uint8_t seen_low;
    uint8_t seen_high;

    SYSCFG_DL_init();
    DL_WWDT_restart(WWDT0_INST);
    BSP_Adc_Init();
    previous = read_gray_bits();
    seen_high = previous;
    seen_low = (uint8_t)(~previous) & GRAY_MASK;

    write_string("\r\nGRAY7 DIGITAL TEST v1\r\n");
    write_string("CH1..CH7=PB20,PB24,PB25,PA24,PA25,PA26,PA27 (LEFT..RIGHT)\r\n");
    write_string("FORMAT:$GRAY,ms,ch1..ch7,bits,changed,seen0,seen1,edge1..7\r\n");

    for (;;) {
        uint8_t bits;
        uint8_t changed;

        delay_cycles((CPUCLK_FREQ / 1000u) * SAMPLE_PERIOD_MS);
        DL_WWDT_restart(WWDT0_INST);
        now_ms += SAMPLE_PERIOD_MS;
        report_elapsed += SAMPLE_PERIOD_MS;
        heartbeat_elapsed += SAMPLE_PERIOD_MS;

        bits = read_gray_bits();
        changed = (uint8_t)(bits ^ previous) & GRAY_MASK;
        seen_high |= bits;
        seen_low |= (uint8_t)(~bits) & GRAY_MASK;

        for (uint8_t i = 0u; i < BSP_ADC_CCD_COUNT; ++i) {
            if ((changed & (uint8_t)(1u << i)) != 0u && edges[i] != UINT16_MAX) {
                edges[i]++;
            }
        }
        previous = bits;

        if (report_elapsed >= REPORT_PERIOD_MS) {
            report_elapsed = 0u;
            report(now_ms, bits, changed, seen_low, seen_high, edges);
        }
        if (heartbeat_elapsed >= HEARTBEAT_MS) {
            heartbeat_elapsed = 0u;
            DL_GPIO_togglePins(STATUS_LED_PORT, STATUS_LED_PIN);
        }
    }
}
