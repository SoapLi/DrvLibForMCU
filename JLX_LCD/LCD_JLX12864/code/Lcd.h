#ifndef _LCD_H
#define _LCD_H

#include "LF_DataType.h"

#define			LINE1		0x80 //第一行起始地址，下同
#define			LINE2 		0x90
#define			LINE3 		0x88
#define			LINE4 		0x98

//以下是点亮你的屏幕必要的六个函数，不能再少了，想添加其他功能就得再加其他函数
void IO_Init(void); //必要的IO口初始化
void CheckBusy(void); //检查忙/闲状态
void LCD_wdat(uchar dat); //写数据
void LCD_wcmd(uchar com); //写命令
void LCD_Init(void); //LCD初始化
void LCD_Wmessage(uchar* message, uchar len, uchar addrx, uchar addry); //向屏幕里写入字串

#endif
