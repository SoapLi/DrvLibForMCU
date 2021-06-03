#include "adc_service.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "../components/cus_esp_adc_cal/include/esp_adc_cal.h"

#include "esp_log.h"
#include "esp_system.h"

#define TAG "LJH_ADC"

// ADC所接的通道  GPIO34 if ADC1  = ADC1_CHANNEL_6
#define ADC1_TEST_CHANNEL ADC1_CHANNEL_6 
// ADC斜率曲线
static esp_adc_cal_characteristics_t *adc_chars;
// 参考电压
#define DEFAULT_VREF				3300			//Use adc2_vref_to_gpio() to obtain a better estimate


static int adc_init_done = 0;


int get_adc_init_sta(void)
{
	return adc_init_done;
}

void adc_Init(void)
{
	adc1_config_width(ADC_WIDTH_BIT_12);// 12位分辨率
	adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN_DB_11);// 电压输入衰减
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));	// 为斜率曲线分配内存
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
	adc_init_done = 1;
}

float get_adc_val(void)
{
	uint32_t read_raw;
	read_raw = adc1_get_raw(ADC1_TEST_CHANNEL);// 采集ADC原始值
	uint32_t voltage  = esp_adc_cal_raw_to_voltage(read_raw, adc_chars);//通过一条斜率曲线把读取adc1_get_raw()的原始数值转变成了mV
	ESP_LOGI(TAG, "ADC raw: 0x%x   voltage: %dmV\n", read_raw, voltage);
	return (float)voltage;
}

void adc_task(void *pvParameters)
{
	while(1)
	{

		get_adc_val();
		
		vTaskDelay(100 / portTICK_PERIOD_MS);	
	}
}