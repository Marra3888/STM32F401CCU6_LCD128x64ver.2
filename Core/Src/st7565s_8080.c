#include "st7565s_8080.h"

// Быстрая запись 8-бит на PB0..PB7
static inline void LCD_BUS_WRITE(uint8_t v) {
  uint32_t odr = LCD_DATA_GPIO->ODR;
  odr &= ~0x00FFu;
  odr |= v;
  LCD_DATA_GPIO->ODR = odr;
}

static inline void CS_LOW(void)  { HAL_GPIO_WritePin(LCD_CS_GPIO,  LCD_CS_PIN,  GPIO_PIN_RESET); }
static inline void CS_HIGH(void) { HAL_GPIO_WritePin(LCD_CS_GPIO,  LCD_CS_PIN,  GPIO_PIN_SET);   }
static inline void RS_CMD(void)  { HAL_GPIO_WritePin(LCD_RS_GPIO,  LCD_RS_PIN,  GPIO_PIN_RESET); }
static inline void RS_DATA(void) { HAL_GPIO_WritePin(LCD_RS_GPIO,  LCD_RS_PIN,  GPIO_PIN_SET);   }

static inline void WR_STROBE(void){
  HAL_GPIO_WritePin(LCD_WR_GPIO, LCD_WR_PIN, GPIO_PIN_RESET);
  __NOP(); __NOP(); // >80 нс
  HAL_GPIO_WritePin(LCD_WR_GPIO, LCD_WR_PIN, GPIO_PIN_SET);
}

static void ST7565S_Reset(void){
  HAL_GPIO_WritePin(LCD_RES_GPIO, LCD_RES_PIN, GPIO_PIN_RESET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(LCD_RES_GPIO, LCD_RES_PIN, GPIO_PIN_SET);
  HAL_Delay(5);
}

static void ST7565S_WriteByte(uint8_t v){
  LCD_BUS_WRITE(v);
  WR_STROBE();
}

void ST7565S_WriteCmd(uint8_t cmd){
  RS_CMD();
  CS_LOW();
  ST7565S_WriteByte(cmd);
  CS_HIGH();
}

void ST7565S_WriteData(uint8_t data){
  RS_DATA();
  CS_LOW();
  ST7565S_WriteByte(data);
  CS_HIGH();
}

void ST7565S_SetAddress(uint8_t page, uint8_t col){
  col += LCD_COL_OFFSET;
  ST7565S_WriteCmd(0xB0 | (page & 0x07));          // Page 0..7
  ST7565S_WriteCmd(0x10 | ((col >> 4) & 0x0F));    // Col high
  ST7565S_WriteCmd(0x00 | (col & 0x0F));           // Col low
}

void ST7565S_Init(void){
  // Линии в безопасные состояния (на случай, если CubeMX не успел)
  HAL_GPIO_WritePin(LCD_RD_GPIO,  LCD_RD_PIN,  GPIO_PIN_SET);   // /RD=HIGH
  HAL_GPIO_WritePin(LCD_WR_GPIO,  LCD_WR_PIN,  GPIO_PIN_SET);   // /WR=HIGH
  HAL_GPIO_WritePin(LCD_CS_GPIO,  LCD_CS_PIN,  GPIO_PIN_SET);   // /CS=HIGH
  HAL_GPIO_WritePin(LCD_RES_GPIO, LCD_RES_PIN, GPIO_PIN_SET);   // /RES=HIGH
  HAL_GPIO_WritePin(LCD_RS_GPIO,  LCD_RS_PIN,  GPIO_PIN_RESET); // RS=0

  ST7565S_Reset();

  // Базовая инициализация
  ST7565S_WriteCmd(0xAE);       // Display OFF
  ST7565S_WriteCmd(0xE2);       // SW Reset
  HAL_Delay(2);

  ST7565S_WriteCmd(0x40);       // Start line 0

#if LCD_MIRROR_X
  ST7565S_WriteCmd(0xA1);       // ADC reverse
#else
  ST7565S_WriteCmd(0xA0);       // ADC normal
#endif

#if LCD_MIRROR_Y
  ST7565S_WriteCmd(0xC8);       // COM reverse
#else
  ST7565S_WriteCmd(0xC0);       // COM normal
#endif

  ST7565S_WriteCmd(0xA2);       // Bias 1/9 (для 64 строк)
  ST7565S_WriteCmd(0x2F);       // Power: Booster+Regulator+Follower ON

  // Booster ratio (опц.)
  ST7565S_WriteCmd(0xF8);
  ST7565S_WriteCmd(LCD_BOOSTER_RATIO); // 0x00=4x, 0x01=5x, 0x03=6x

  // Резисторный делитель (контраст)
  ST7565S_WriteCmd(0x20 | (LCD_RESISTOR_RATIO & 0x07));

  // Electronic Volume (контраст)
  ST7565S_WriteCmd(0x81);
  ST7565S_WriteCmd(LCD_ELECT_VOLUME);

  ST7565S_WriteCmd(0xA6);       // Normal (не инверсия)
  ST7565S_WriteCmd(0xA4);       // RAM display (не All Points On)
  ST7565S_WriteCmd(0xAF);       // Display ON

  // Очистка
  for (uint8_t p = 0; p < LCD_PAGES; p++){
    ST7565S_SetAddress(p, 0);
    for (uint8_t x = 0; x < LCD_WIDTH; x++) ST7565S_WriteData(0x00);
  }
}

void ST7565S_Fill(uint8_t pattern){
  for (uint8_t p = 0; p < LCD_PAGES; p++){
    ST7565S_SetAddress(p, 0);
    for (uint8_t x = 0; x < LCD_WIDTH; x++) ST7565S_WriteData(pattern);
  }
}

void ST7565S_DrawBuffer(const uint8_t* buf){
  for (uint8_t p = 0; p < LCD_PAGES; p++){
    ST7565S_SetAddress(p, 0);
    const uint8_t* line = buf + p*LCD_WIDTH;
    for (uint8_t x = 0; x < LCD_WIDTH; x++) ST7565S_WriteData(line[x]);
  }
}

void ST7565S_SetContrast(uint8_t ev) { // 0..63 (0x00..0x3F)
  if (ev > 0x3F) ev = 0x3F;
  ST7565S_WriteCmd(0x81);
  ST7565S_WriteCmd(ev);
}

void ST7565S_SetResistorRatio(uint8_t rr) { // 0..7 => 0x20..0x27
  ST7565S_WriteCmd(0x20 | (rr & 0x07));
}

void ST7565S_SetBias(uint8_t bias19) { // 1 = 1/9, 0 = 1/7
  ST7565S_WriteCmd(bias19 ? 0xA2 : 0xA3);
}

void ST7565S_SetBooster(uint8_t ratio) { // 0x00=4x, 0x01=5x, 0x03=6x
  ST7565S_WriteCmd(0xF8);
  ST7565S_WriteCmd(ratio & 0x03);
}