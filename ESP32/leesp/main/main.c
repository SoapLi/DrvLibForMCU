/* LVGL Example project */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_app_format.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_freertos_hooks.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "driver/gpio.h"
#include "../build/config/sdkconfig.h"
#include "../components/cus_esp_mqtt/esp-mqtt/include/mqtt_client.h"
#include "../components/cus_freemodbus/common/include/mbcontroller.h"
#include "ui_boot_service.h"
#include "ui_service.h"
#include "wifi_service.h"
#include "mqtt_service.h"
#include "mbs_service.h"
#include "weather_http_service.h"
#include "sntp_service.h"
#include "oled.h"
#include "adc_service.h"
#include "ota_service.h"

#define TAG "LJH_MAIN"
//#define OLED_TEST

#define FACTORY_BOOT


void oled_time_task(void *pvParameters)
{
	sntp_info sntp;
	while (1)
	{
		esp_get_local_time(&sntp);
		oled_show_str(0, 20, (char *)&sntp.real_time[8], &Font_7x10, 1);
		if(get_wifi_sta()){
			oled_show_str(0, 0, (char *)"Wifi Connected  ", &Font_7x10, 1);
		}else{
			oled_show_str(0, 0, (char *)"Wifi Disconnect!", &Font_7x10, 1);
		}
		oled_update_screen();
		get_adc_val();
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}


// 主函数
void app_main()
{

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	ESP_LOGI(TAG, "\r\n\r\n\r\n!!!!!!!!!!!!!APP is start!~~~~~~~~~~~~~~\r\n\r\n\r\n");

#ifdef OLED_TEST
	adc_Init();
	oled_init();
	oled_claer();
	oled_show_str(0, 0, (char *)"WiFi Connectting..", &Font_7x10, 1);
	oled_update_screen();
	wifi_init();
	oled_show_str(0, 0, (char *)"SNTP Connectting..", &Font_7x10, 1);
	esp_initialize_sntp();
	esp_wait_sntp_sync();
	xTaskCreate(oled_time_task, "oled_time_task", 4096, NULL, 4, NULL);

#else
	
	#ifdef FACTORY_BOOT
	xTaskCreatePinnedToCore(guiBootTask, "guiBootTask", 4096 * 2, NULL, 2, NULL, 1);
	xTaskCreate(ota_service, "ota_service", 4096, NULL, 4, NULL);
	wifi_init();

	#else
	xTaskCreatePinnedToCore(guiTask, "guiTask", 4096 * 2, NULL, 2, NULL, 1);
	wifi_init();
	//xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL,4, NULL);

	#endif

	// xTaskCreate(mbs_task, "mbs_task", 4096, NULL,4, NULL);
	// xTaskCreate(http_weather_task, "http_weather_task", 4096, NULL, 4, NULL);
	// esp_initialize_sntp();
	// esp_wait_sntp_sync();
	//xTaskCreate(ota_service, "ota_service", 4096, NULL, 5, NULL);
	//adc_Init();
	// get_adc_val();
	// get_adc_val();
	// get_adc_val();
	// xTaskCreate(adc_task, "adc_task", 4096, NULL, 5, NULL);
#endif
}
