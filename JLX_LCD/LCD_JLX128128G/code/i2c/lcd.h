#ifndef _LCD_H
#define _LCD_H
#include "sys.h"
#include "delay.h"

#define RES PHout(10)
#define CLK PHout(13)
#define SDA PHout(14)

void LCD_Init(void);
void clear_screen(void);
void display_string_8x16(u8 page,u8 column,u8 *text);
void test_screen();

#endif
