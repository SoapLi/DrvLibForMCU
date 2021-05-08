#include "Lcd.h"
#include "main.h"
#include "Sys.h"

#define RS PGout(0) //������/������ƿ�
#define RW PGout(1) //��/д���ƿڣ�д�Ƚϳ���
#define EN PGout(2) //ʹ�ܿ�
#define RST PGout(3) //ʹ�ܿ�

ushort temp;

//LCD��ʼ��
void LCD_Init()
{
	RST = 1;	
	HAL_Delay(5);
	LCD_wcmd(0x30);//�����趨������ָ�
	HAL_Delay(5);
	LCD_wcmd(0x0C);//��ʾ�����ع��
	HAL_Delay(5);
	LCD_wcmd(0x01);//�����ʾ
}

//æ�ж�
void CheckBusy(void)
{
	uchar status;
	RS=0;
	RW=1; //�������ݣ�RW=1
	GPIOD->ODR = 0xFF;
	do
	{
		EN = 1;
		HAL_Delay(5);
		status = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_15);//�ж�BFλ
	}while(status & 0x80);
	EN=0;
}

//LCDд����
void LCD_wcmd(uchar cmd)
{
	CheckBusy();
	RS=0;
	RW=0;
	HAL_Delay(5);
	temp=(temp&0x00ff)|cmd<<8;//temp�ĵͰ�λ�����cmd�Ž�ȥ
	GPIOD->ODR = temp;//�����ݷ���A�˿ڣ�Ҳ����LCD�����ݿڣ�
	EN=1;			//ʹ��λ����
	HAL_Delay(10);	//10msӦ���ܷ�������
	EN=0;			//ʹ��λ�ر�
}

//LCDд����
void LCD_wdat(uchar dat)
{
	CheckBusy();
	RS=1;
	RW=0;
	HAL_Delay(5);
	temp=(temp&0x00ff)|dat<<8;//temp�ĵͰ�λ�����cmd�Ž�ȥ
	GPIOD->ODR = temp;		//�����ݷ���A�˿ڣ�Ҳ����LCD�����ݿڣ�
	EN=1;					//ʹ��λ����
	HAL_Delay(10);			//10msӦ���ܷ�������
	EN=0;					//ʹ��λ�ر�
}

//��LCD12864��д��һ�����ݣ���Ϊ�㲻����ÿ��ֻ����һ�ֽ����ݣ�
void LCD_Wmessage(uchar* message, uchar len, uchar addrx, uchar addry)
{
	uchar index = 0;
	
	LCD_wcmd(addrx + addry);	//Ҫ��ʾ��λ�ã�������������ʾ��LCD����һ�У��ͰѸ��е���ʼ��ַͨ��д����ķ�ʽ���ͳ�ȥ�������棬��ľ��
	while((index < len) && (*(message+index) != '\0'))			//����жϺܹؼ����ж����������û�з���
	{
		LCD_wdat(*(message+index)); 	//�ں˻��Ƿ��ֽں���
		index++; 				//ָ��ͦ���õġ���
	}
}

