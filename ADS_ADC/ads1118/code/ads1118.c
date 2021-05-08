/*
 * ads1118.c
 *
 *  Created on: Apr 26, 2021
 *      Author: LJH
 */

#include "ads1118.h"


ADS_InitTypeDef adsConfigReg = {
	.stru.NOP = DATA_VALID,
	.stru.PULLUP = PULL_UP_DIS,
	.stru.TS_MODE = ADC_MODE,
	.stru.DR = DR_860_SPS,
	.stru.MODE = SIGNLE_SHOT,
	.stru.PGA = PGA_6144,
	.stru.MUX = AINPN_0_GND,
	.stru.OS = SINGLE_CONVER_START,
	.stru.RESV = CONFIG_BIT_RESV
};

uint8_t ads1118_write_byte(uint8_t byte)
{
	uint8_t ret_data;
	HAL_SPI_TransmitReceive(&hspi4, &byte, &ret_data,  1, 1000);
	return ret_data;
}

uint16_t ads1118_read_write_reg(uint16_t CofigReg)
{
	uint8_t highByte;
	uint8_t lowByte;
	highByte = ads1118_write_byte((uint8_t)(CofigReg >> 8));
	lowByte = ads1118_write_byte((uint8_t)CofigReg);
	return ((uint16_t)lowByte | ((uint16_t)highByte << 8));
}

float ads1118_raw_to_vol(uint16_t adc_raw)
{
	float vol_mv;
	if(adc_raw & 0x8000)
	{
		return 0;
	}
	switch(adsConfigReg.stru.PGA)
	{
	case PGA_6144:
		vol_mv = adc_raw * ADS1118_CONST_6_144V_LSB_mV;
		break;
	case PGA_4096:
		vol_mv = adc_raw * ADS1118_CONST_4_096V_LSB_mV;
		break;
	case PGA_2048:
		vol_mv = adc_raw * ADS1118_CONST_2_048V_LSB_mV;
		break;
	case PGA_1024:
		vol_mv = adc_raw * ADS1118_CONST_1_024V_LSB_mV;
		break;
	case PGA_512:
		vol_mv = adc_raw * ADS1118_CONST_0_512V_LSB_mV;
		break;
	case PGA_256:
		vol_mv = adc_raw * ADS1118_CONST_0_256V_LSB_mV;
		break;
	default:
		break;
	}
	return vol_mv / 1000;
}

float ads1118_get_vol(uint8_t channel)
{
	float adc_vol = 0;
	adsConfigReg.stru.NOP = DATA_VALID;
	adsConfigReg.stru.MUX = channel;
	ADS1118_ENABLE;
	HAL_Delay(1);
	ads1118_read_write_reg(adsConfigReg.word);
	ADS1118_DISABLE;
	HAL_Delay(1);
	ADS1118_ENABLE;
	HAL_Delay(1);
	adsConfigReg.stru.NOP = DATA_INVALID;
	adc_vol = ads1118_raw_to_vol(ads1118_read_write_reg(adsConfigReg.word));
	HAL_Delay(1);
	ADS1118_DISABLE;
	HAL_Delay(1);
	return adc_vol;
}














































