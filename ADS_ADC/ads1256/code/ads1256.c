/*
 * ads1256.c
 *
 *  Created on: Jan 20, 2021
 *      Author: LJH
 */
#include "ServiceMain.h"
#include "ads1256.h"
#include "usart.h"
#include "delay.h"
#include <stdio.h>
#include "mcu_info.h"
#include "marco.h"

adc_cali_para_t adc_cali_para = {
	.channel[0].k = 1,
	.channel[0].b = 0,
	.channel[1].k = 1,
	.channel[1].b = 0,
	.channel[2].k = 1,
	.channel[2].b = 0,
	.channel[3].k = 1,
	.channel[3].b = 0,
	.channel[4].k = 1,
	.channel[4].b = 0,
	.channel[5].k = 1,
	.channel[5].b = 0,
	.channel[6].k = 1,
	.channel[6].b = 0,
	.channel[7].k = 1,//test only
	.channel[7].b = 0,
};

static void spiWriteByte(u8 txData) {

	u8 tempData = 0x00;
	HAL_SPI_TransmitReceive(&hspi4, &txData, &tempData, 1, 100);

}

static void spiWriteRegData(u8 regAdd, u8 regData) {

	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	spiWriteByte(WREG | (regAdd & 0x0F));
	spiWriteByte(0x00);
	spiWriteByte(regData);
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);

}

static void ads1256HardReset(void) {
	////hard reset
	HAL_GPIO_WritePin(AD_RESET_GPIO_Port, AD_RESET_Pin, GPIO_PIN_SET);
	delay_us(50);
	HAL_GPIO_WritePin(AD_RESET_GPIO_Port, AD_RESET_Pin, GPIO_PIN_RESET);
	delay_us(50);
	HAL_GPIO_WritePin(AD_RESET_GPIO_Port, AD_RESET_Pin, GPIO_PIN_SET);
}

static void ads1256SHardSync(void) {
	//hard sync
	HAL_GPIO_WritePin(AD_SYNC_GPIO_Port, AD_SYNC_Pin, GPIO_PIN_RESET);
	delay_us(50);
	HAL_GPIO_WritePin(AD_SYNC_GPIO_Port, AD_SYNC_Pin, GPIO_PIN_SET);
}

static void ads1256SelfCal(void) {
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);
	spiWriteByte(SELFCAL); //0xF0
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);
	delay_us(10);
}

static void ads1256SetChannel(u8 channel) {
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);
	spiWriteRegData(MUX, channel | MUXN_AINCOM); //single mode
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);
}

static void ads1256InitConfig(u8 channel, int buff_on) {
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	delay_us(10);
	if (buff_on) {
		spiWriteRegData(STATUS, 0x06);       //calibra and buf
	} else {
		spiWriteRegData(STATUS, 0x04);      //calibra and no buf
	}
	delay_us(10);
	spiWriteRegData(MUX, channel | MUXN_AINCOM); //single mode
	delay_us(10);
	//set adc gain
	spiWriteRegData(ADCON, GAIN_1);
	delay_us(10);
	//set adc simple rate
	spiWriteRegData(DRATE, RATE_30000);
	delay_us(10);
	//set io state
	spiWriteRegData(IO, 0x00);
	delay_us(10);
}



static u8 spiReadByte(void) {
	u8 tempDataT = 0xff;
	u8 tempData = 0x00;
	HAL_SPI_TransmitReceive(&hspi4, &tempDataT, &tempData, 1, 100);
	return tempData;
}

static double ads1256ReadValue(u8 channel) {
	int32_t sum = 0;
	ads1256SetChannel(channel);
	delay_us(5);
	ads1256SelfCal();
	delay_us(5);
	spiWriteRegData(DRATE, RATE_30000);
	delay_us(10);
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin));
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);
	spiWriteByte(SYNC);
	delay_us(5);
	spiWriteByte(WAKEUP);
	delay_us(5);
	spiWriteByte(RDATA);
	delay_us(25);

	sum |= (spiReadByte() << 16);
	sum |= (spiReadByte() << 8);
	sum |= (spiReadByte());
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);
	if (sum > 0x7fffff)
		sum -= 0x1000000;

	return (float) (sum * 5.0 / 8388607);;

}

static int32_t ads1256ReadReg(u8 regAdd) {

	int32_t regValue = 0;
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_RESET);
	spiWriteByte(SYNC);
	delay_us(5);
	spiWriteByte(WAKEUP);
	delay_us(5);
	spiWriteByte(RREG | (regAdd & 0xF));
	spiWriteByte(0x00);
	delay_us(25);
	regValue |= (spiReadByte());
	HAL_GPIO_WritePin(AD_CS_GPIO_Port, AD_CS_Pin, GPIO_PIN_SET);
	if (regValue > 0x7fffff)
		regValue -= 0x1000000;
	//printf("REG : 0x%0x\r\n", regValue);
	return regValue;

}

double ads1256ReadValueV2(u8 channel, int buff_on,int filter) {
	double ret;
	while (HAL_GPIO_ReadPin(AD_READY_GPIO_Port, AD_READY_Pin))
		;
	HAL_Delay(1);
	if (buff_on) {
		spiWriteRegData(STATUS, 0x06);       //calibra and buf
	} else {
		spiWriteRegData(STATUS, 0x04);      //calibra and no buf
	}
	double sum = 0;
	for(int i = 0;i<filter;i++)
	{
		HAL_Delay(1);
		sum = sum + ads1256ReadValue(channel);
		HAL_Delay(1);
	}
	//y = kx + b;
	ret = (adc_cali_para.channel[channel/0x10].k) * (sum/filter) + (adc_cali_para.channel[channel/0x10].b);
	return ret;
}

void ads1256Init(u8 channel, int buff_on) {
	//spi raw data : 0xF0 -> 0x50 0x00 0x06 -> 0x51 0x00 0x01 -> 0x52 0x00 0x00 -> 0x53 0x00 0xf0 -> 0x54 0x00 0x00 -> 0xF0
	ads1256HardReset();
	ads1256SelfCal();
	ads1256InitConfig(channel, buff_on);
	ads1256SelfCal();
	ads1256SHardSync();
}

void TEST_ads1256_channnel(void) {
	for (int i = 0; i < 8; i++) {
		printf("channel[%d]:%fV   ", i,ads1256ReadValueV2(MUXP_AIN0 + 0x10 * i, BUFF_ON,20));
	}
	printf("\r\n");
}

