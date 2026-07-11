#ifndef SSD1306_MSPM0_H
#define SSD1306_MSPM0_H

#include <stdbool.h>
#include <stdint.h>

#include "ti_msp_dl_config.h"

#define SSD1306_I2C_ADDRESS_DEFAULT (0x3Cu)
#define SSD1306_I2C_ADDRESS_ALT     (0x3Du)
#define SSD1306_WIDTH               (128u)
#define SSD1306_HEIGHT              (64u)
#define SSD1306_TEXT_COLUMNS        (21u)
#define SSD1306_TEXT_ROWS           (8u)

bool SSD1306_MSPM0_Init(I2C_Regs *i2c, uint8_t address);
bool SSD1306_MSPM0_Clear(void);
bool SSD1306_MSPM0_WriteString(uint8_t row, uint8_t col, const char *text);
bool SSD1306_MSPM0_WriteStringPadded(uint8_t row, uint8_t col, const char *text);

#endif
