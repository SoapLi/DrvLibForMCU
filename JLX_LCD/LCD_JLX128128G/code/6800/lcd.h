#ifndef _LCD_H
#define _LCD_H
#include "sys.h"
#include "delay.h"

#define RS PHout(13)
#define RW PHout(14)
#define RD PHout(9) //EN
#define RES PHout(10)
#define CSL PHout(11)

void LCD_Init(void);
void clear_screen(void);
void display_string_8x16(u8 page,u8 column,u8 *text);
void test_screen();

#endif
