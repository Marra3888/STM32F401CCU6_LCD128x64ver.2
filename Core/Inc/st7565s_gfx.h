/*
 * st7565s_gfx.h
 *
 *  Created on: Nov 24, 2025
 *      Author: Zver
 */

#ifndef INC_ST7565S_GFX_H_
#define INC_ST7565S_GFX_H_

#pragma once
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Использует тот же фреймбуфер, что и у вас: 128 * (64/8) = 1024 байта,
// с раскладкой страниц (по 8 вертикальных пикселей на байт).
#ifndef LCD_WIDTH
#define LCD_WIDTH   128
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT   64
#endif

// Цвет: 1 = пиксель включен, 0 = выключен
void GFX_DrawPixel(uint8_t* fb, int x, int y, bool color);

// Символ 5x7 (моноширинный), с шагом 6 пикселей (1 пиксель — межсимвольный пробел)
void GFX_DrawChar(uint8_t* fb, int x, int y, char c, bool color);

// Строка из ASCII (печатаемые 0x20..0x7E), перенос по '\n'
void GFX_DrawString(uint8_t* fb, int x, int y, const char* s, bool color);

// st7565s_gfx.h
void GFX_DrawCharScaled(uint8_t* fb, int x, int y, char c, bool color, uint8_t scale);
void GFX_DrawStringScaled(uint8_t* fb, int x, int y, const char* s, bool color, uint8_t scale);
int  GFX_TextWidth(const char* s, uint8_t scale); // ширина строки в пикселях

// Рисуем 6x8 глиф (колонки, bit0 = верхний пиксель), с масштабом 1..N
void GFX_DrawGlyph6x8(uint8_t* fb, int x, int y, const uint8_t col[6], bool color, uint8_t scale);

// UTF-8 строка (ASCII + кириллица D0/D1), scale=1 или 2
void GFX_DrawStringUTF8_RU(uint8_t* fb, int x, int y, const char* utf8, bool color, uint8_t scale);

// Глиф 6x8 (6 столбцов, bit0 = верхний пиксель), масштаб 1..N
void GFX_DrawGlyph6x8(uint8_t* fb, int x, int y, const uint8_t col[6], bool color, uint8_t scale);

// UTF-8 строка (ASCII + кириллица D0/D1), scale=1..N
void GFX_DrawStringUTF8_RU(uint8_t* fb, int x, int y, const char* utf8, bool color, uint8_t scale);

// Ширина UTF-8 строки в пикселях (6 px на символ с учётом scale, до \n)
int GFX_TextWidthUTF8_RU(const char* utf8, uint8_t scale);

#endif /* INC_ST7565S_GFX_H_ */
