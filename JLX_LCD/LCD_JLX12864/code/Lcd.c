#include "Lcd.h"
#include "main.h"
#include "Sys.h"

#define RS PGout(0) //发数据/命令控制口
#define RW PGout(1) //读/写控制口，写比较常用
#define EN PGout(2) //使能口
#define RST PGout(3) //使能口

ushort temp;

//LCD初始化
void LCD_Init()
{
	RST = 1;	
	HAL_Delay(5);
	LCD_wcmd(0x30);//功能设定：基本指令集
	HAL_Delay(5);
	LCD_wcmd(0x0C);//显示开，关光标
	HAL_Delay(5);
	LCD_wcmd(0x01);//清除显示
}

//忙判断
void CheckBusy(void)
{
	uchar status;
	RS=0;
	RW=1; //读出数据，RW=1
	GPIOD->ODR = 0xFF;
	do
	{
		EN = 1;
		HAL_Delay(5);
		status = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_15);//判断BF位
	}while(status & 0x80);
	EN=0;
}

//LCD写命令
void LCD_wcmd(uchar cmd)
{
	CheckBusy();
	RS=0;
	RW=0;
	HAL_Delay(5);
	temp=(temp&0x00ff)|cmd<<8;//temp的低八位清零后将cmd放进去
	GPIOD->ODR = temp;//将数据发到A端口（也就是LCD的数据口）
	EN=1;			//使能位开启
	HAL_Delay(10);	//10ms应该能发送完了
	EN=0;			//使能位关闭
}

//LCD写数据
void LCD_wdat(uchar dat)
{
	CheckBusy();
	RS=1;
	RW=0;
	HAL_Delay(5);
	temp=(temp&0x00ff)|dat<<8;//temp的低八位清零后将cmd放进去
	GPIOD->ODR = temp;		//将数据发到A端口（也就是LCD的数据口）
	EN=1;					//使能位开启
	HAL_Delay(10);			//10ms应该能发送完了
	EN=0;					//使能位关闭
}

//向LCD12864中写入一行数据（因为你不可能每次只发送一字节数据）
void LCD_Wmessage(uchar* message, uchar len, uchar addrx, uchar addry)
{
	uchar index = 0;
	
	LCD_wcmd(addrx + addry);	//要显示的位置，你想让内容显示在LCD的哪一行，就把该行的起始地址通过写命令的方式发送出去，很神奇，有木有
	while((index < len) && (*(message+index) != '\0'))			//这个判断很关键，判断你的内容有没有发完
	{
		LCD_wdat(*(message+index)); 	//内核还是发字节函数
		index++; 				//指针挺好用的。。
	}
}

