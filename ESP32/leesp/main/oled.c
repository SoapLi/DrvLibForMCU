/*
* @file         oled.c 
* @brief        ESP32����OLED-I2C
* @details      �û�Ӧ�ó��������ļ�,�û�����Ҫʵ�ֵĹ����߼����ǴӸ��ļ���ʼ���ߴ���
* @author       �����Ŷ� 
* @par Copyright (c):  
*               �������߿����Ŷӣ�QQȺ��671139854
*/
/* 
=============
ͷ�ļ�����
=============
*/
#include "oled.h"
//#include "oledfont.h"
#include "string.h"
#include "stdlib.h"
#include "fonts.h"
/*
===========================
ȫ�ֱ�������
=========================== 
*/
//OLED����128*64bit
static uint8_t g_oled_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
//OLEDʵʱ��Ϣ
static SSD1306_t oled;
//OLED�Ƿ�������ʾ��1��ʾ��0�ȴ�
static bool is_show_str = 0;
/*
===========================
��������
=========================== 
*/

/** 
 * oled_i2c ��ʼ��
 * @param[in]   NULL
 * @retval      
 *              NULL                              
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 *               Ver0.0.2:
                     hx-zsj, 2018/08/07, ͳһ��̷��\n 
 */
void i2c_init(void)
{
	//ע�Ͳο�sht30֮i2c�̳�
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_OLED_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = I2C_OLED_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	i2c_param_config(I2C_OLED_MASTER_NUM, &conf);
	i2c_driver_install(I2C_OLED_MASTER_NUM, conf.mode, 0, 0, 0);
}

/** 
 * ��oledд����
 * @param[in]   command
 * @retval      
 *              - ESP_OK                              
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 *               Ver0.0.2:
                     hx-zsj, 2018/08/07, ͳһ��̷��\n 
 */

int oled_write_cmd(uint8_t command)
{
	//ע�Ͳο�sht30֮i2c�̳�
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ret = i2c_master_start(cmd);
	ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR | WRITE_BIT, ACK_CHECK_EN); 
	ret = i2c_master_write_byte(cmd, WRITE_CMD, ACK_CHECK_EN);
	ret = i2c_master_write_byte(cmd, command, ACK_CHECK_EN);
	ret = i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK) 
	{
		return ret;
	}
	return ret;
}

/** 
 * ��oledд����
 * @param[in]   data
 * @retval      
 *              - ESP_OK                              
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
int oled_write_data(uint8_t data)
{
	//ע�Ͳο�sht30֮i2c�̳�
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ret = i2c_master_start(cmd);
	ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR | WRITE_BIT, ACK_CHECK_EN);
	ret = i2c_master_write_byte(cmd, WRITE_DATA, ACK_CHECK_EN);
	ret = i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
	ret = i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 100 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK) 
	{
		return ret;
	}
	return ret;
}
/** 
 * ��oledд������
 * @param[in]   data   Ҫд�������
 * @param[in]   len     ���ݳ���
 * @retval      
 *              - ESP_OK                              
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
int oled_write_long_data(uint8_t *data, uint16_t len)
{
	//ע�Ͳο�sht30֮i2c�̳�
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ret = i2c_master_start(cmd);
	ret = i2c_master_write_byte(cmd, OLED_WRITE_ADDR | WRITE_BIT, ACK_CHECK_EN);
	ret = i2c_master_write_byte(cmd, WRITE_DATA, ACK_CHECK_EN);
	ret = i2c_master_write(cmd, data, len, ACK_CHECK_EN);
	ret = i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_OLED_MASTER_NUM, cmd, 10000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	if (ret != ESP_OK) 
	{
		return ret;
	}
	return ret;    
}

/** 
 * ��ʼ�� oled
 * @param[in]   NULL
 * @retval      
 *              NULL                            
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_init(void)
{
	//i2c��ʼ��
	i2c_init();
	//oled����
	oled_write_cmd(TURN_OFF_CMD);
	oled_write_cmd(0xAE); /*display off*/
	oled_write_cmd(0x00); /*set lower column address*/ 
	oled_write_cmd(0x10); /*set higher column address*/
	oled_write_cmd(0x00); /*set display start line*/ 
	oled_write_cmd(0xB0); /*set page address*/ 
	oled_write_cmd(0x81); /*contract control*/ 
	oled_write_cmd(0xff); /*128*/ 
	oled_write_cmd(0xA1); /*set segment remap*/ 
	oled_write_cmd(0xA6); /*normal / reverse*/ 
	oled_write_cmd(0xA8); /*multiplex ratio*/ 
	oled_write_cmd(0x1F); /*duty = 1/32*/ 
	oled_write_cmd(0xC8); /*Com scan direction*/ 
	oled_write_cmd(0xD3); /*set display offset*/ 
	oled_write_cmd(0x00); 
	oled_write_cmd(0xD5); /*set osc division*/ 
	oled_write_cmd(0x80); 
	oled_write_cmd(0xD9); /*set pre-charge period*/ 
	oled_write_cmd(0x1f); 
	oled_write_cmd(0xDA); /*set COM pins*/ 
	oled_write_cmd(0x00); 
	oled_write_cmd(0xdb); /*set vcomh*/ 
	oled_write_cmd(0x40); 
	oled_write_cmd(0x8d); /*set charge pump enable*/ 
	oled_write_cmd(0x14);
	oled_write_cmd(0xAF); /*display ON*/
}

/** 
 * ���Դ�����ˢ�µ�oled��ʾ��
 * @param[in]   NULL
 * @retval      
 *              NULL                           
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_update_screen(void)
{
	uint8_t line_index;
	for (line_index = 0; line_index < 8; line_index++)
	{
		oled_write_cmd(0xb0 + line_index);
		oled_write_cmd(0x00);
		oled_write_cmd(0x10);
        
		oled_write_long_data(&g_oled_buffer[SSD1306_WIDTH * line_index], SSD1306_WIDTH);
	}
}

/** 
 * ����
 * @param[in]   NULL
 * @retval      
 *              NULL                            
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_claer(void)
{
	//��0����
	memset(g_oled_buffer, SSD1306_COLOR_BLACK, sizeof(g_oled_buffer));
	oled_update_screen();
}
/** 
 * ����
 * @param[in]   NULL
 * @retval      
 *              NULL                            
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_all_on(void)
{
	//��ff����
	memset(g_oled_buffer, 0xff, sizeof(g_oled_buffer));
	oled_update_screen();
}
/** 
 * �ƶ�����
 * @param[in]   x   ��ʾ������ x
 * @param[in]   y   ��ʾȥ���� y
 * @retval      
 *              ����                         
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_gotoXY(uint16_t x, uint16_t y) 
{
	oled.CurrentX = x;
	oled.CurrentY = y;
}
/** 
 * ���Դ�д��
 * @param[in]   x   ����
 * @param[in]   y   ����
 * @param[in]   color   ɫֵ0/1
 * @retval      
 *              - ESP_OK                              
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
void oled_drawpixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) 
{
	if (
		x >= SSD1306_WIDTH ||
		y >= SSD1306_HEIGHT) 
	{
		return;
	}
	if (color == SSD1306_COLOR_WHITE) 
	{
		g_oled_buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else
	{
		g_oled_buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}
/** 
 * ��x��yλ����ʾ�ַ�
 * @param[in]   x    ��ʾ����x 
 * @param[in]   y    ��ʾ����y 
 * @param[in]   ch   Ҫ��ʾ���ַ�
 * @param[in]   font ��ʾ������
 * @param[in]   color ��ɫ  1��ʾ 0����ʾ
 * @retval      
 *              ����                        
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
char oled_show_char(uint16_t x, uint16_t y, char ch, FontDef_t* Font, SSD1306_COLOR_t color) 
{
	uint32_t i, b, j;
	if (SSD1306_WIDTH <= (oled.CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (oled.CurrentY + Font->FontHeight)) 
	{
		return 0;
	}
	if (0 == is_show_str)
	{
		oled_gotoXY(x, y);
	}

	for (i = 0; i < Font->FontHeight; i++) 
	{
		b = Font->data[(ch - 32) * Font->FontHeight + i];
		for (j = 0; j < Font->FontWidth; j++)
		{
			if ((b << j) & 0x8000) 
			{
				oled_drawpixel(oled.CurrentX + j, (oled.CurrentY + i), (SSD1306_COLOR_t) color);
			} 
			else 
			{
				oled_drawpixel(oled.CurrentX + j, (oled.CurrentY + i), (SSD1306_COLOR_t)!color);
			}
		}
	}
	oled.CurrentX += Font->FontWidth;
	if (0 == is_show_str)
	{
		oled_update_screen(); 
	}
	return ch;
}
/** 
 * ��x��yλ����ʾ�ַ��� 
 * @param[in]   x    ��ʾ����x 
 * @param[in]   y    ��ʾ����y 
 * @param[in]   str   Ҫ��ʾ���ַ���
 * @param[in]   font ��ʾ������
 * @param[in]   color ��ɫ  1��ʾ 0����ʾ
 * @retval      
 *              ����                        
 * @par         �޸���־ 
 *               Ver0.0.1:
                     XinC_Guo, 2018/07/18, ��ʼ���汾\n 
 */
char oled_show_str(uint16_t x, uint16_t y, char* str, FontDef_t* Font, SSD1306_COLOR_t color) 
{
	is_show_str = 1;
	oled_gotoXY(x, y);
	while (*str) 
	{
		if (oled_show_char(x, y, *str, Font, color) != *str) 
		{
			is_show_str = 0;
			return *str;
		}
		str++;
	}
	is_show_str = 0;
	oled_update_screen();
	return *str;
}