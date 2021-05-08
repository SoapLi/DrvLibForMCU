#ifndef _LCD_H
#define _LCD_H

#include "LF_DataType.h"

#define			LINE1		0x80 //��һ����ʼ��ַ����ͬ
#define			LINE2 		0x90
#define			LINE3 		0x88
#define			LINE4 		0x98

//�����ǵ��������Ļ��Ҫ���������������������ˣ�������������ܾ͵��ټ���������
void IO_Init(void); //��Ҫ��IO�ڳ�ʼ��
void CheckBusy(void); //���æ/��״̬
void LCD_wdat(uchar dat); //д����
void LCD_wcmd(uchar com); //д����
void LCD_Init(void); //LCD��ʼ��
void LCD_Wmessage(uchar* message, uchar len, uchar addrx, uchar addry); //����Ļ��д���ִ�

#endif
