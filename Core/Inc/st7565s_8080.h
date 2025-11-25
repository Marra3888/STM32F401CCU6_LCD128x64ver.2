#pragma once
#include "stm32f4xx_hal.h"

// Пины (правьте под свою разводку)
#define LCD_DATA_GPIO      GPIOB   // DB0..DB7 на PB0..PB7

#define LCD_RS_GPIO        GPIOA
#define LCD_RS_PIN         GPIO_PIN_8
#define LCD_WR_GPIO        GPIOA
#define LCD_WR_PIN         GPIO_PIN_9
#define LCD_RD_GPIO        GPIOA
#define LCD_RD_PIN         GPIO_PIN_10
#define LCD_CS_GPIO        GPIOA
#define LCD_CS_PIN         GPIO_PIN_11
#define LCD_RES_GPIO       GPIOA
#define LCD_RES_PIN        GPIO_PIN_12

// Геометрия дисплея
#define LCD_WIDTH          128
#define LCD_HEIGHT         64
#define LCD_PAGES          (LCD_HEIGHT/8)

// Смещение колонок внутрь ST7565S (0..4 обычно)
#define LCD_COL_OFFSET     0

// Ориентация
#define LCD_MIRROR_X       0   // 0:A0=0xA0, 1:A0=0xA1
#define LCD_MIRROR_Y       1   // 0:C0=0xC0, 1:C8=0xC8

// Контраст/помпа (подбираются)
#define LCD_BOOSTER_RATIO  0x00  // 0x00:4x, 0x01:5x, 0x03:6x
#define LCD_RESISTOR_RATIO 0x27  // 0x20..0x27
#define LCD_ELECT_VOLUME   0x18  // 0x00..0x3F

// API
void ST7565S_Init(void);
void ST7565S_WriteCmd(uint8_t cmd);
void ST7565S_WriteData(uint8_t data);
void ST7565S_SetAddress(uint8_t page, uint8_t col);
void ST7565S_Fill(uint8_t pattern);
void ST7565S_DrawBuffer(const uint8_t* buf); // 1024 байта