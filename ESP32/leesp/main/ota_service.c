#include "ota_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "nvs_flash.h"
#include "esp_wpa2.h"

#include "esp_ota_ops.h"
#include <sys/socket.h>
#include <esp_http_client.h>
// #include "esp_tls.h"

#define TAG "LJH_OTA_SERVICE"

static char readBuf[1024] = {0};

static int size_wanted = 1;
static int size_readed = 0;

esp_ota_handle_t update_handle = 0;
const esp_partition_t *update_partition = NULL;
esp_http_client_handle_t client;
int flag_ota_start = 0;
int flag_ota_init_sta = 0;

esp_err_t _http_event_handler(esp_http_client_event_t *evt);

esp_http_client_config_t config = {
	.host = "192.168.137.1",
	.port = 9998,
	// .path = "/leesp.bin",
		.path = NULL,
	.event_handler = _http_event_handler,
};
	
int get_size_wanted(void)
{
	return size_wanted;
}

int get_size_readed(void)
{
	return size_readed;
}

int get_ota_init_sta(void)
{
	return flag_ota_init_sta;
}

void set_ota_bin_path(char* path)
{
	config.path = (char*)malloc(sizeof(char)*11);
	memset(config.path,0,sizeof(char)*11);
	strncpy(config.path , path,10);
}

//Http事件处理程序
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	esp_err_t err;
	switch (evt->event_id)
	{
	case HTTP_EVENT_ERROR:
		ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		if (strcmp(evt->header_key, "Content-Length") == 0)
		{
			size_wanted = atoi(evt->header_value);
			ESP_LOGI(TAG, "strcmp SUCCESS!!!  size_wanted : %d\r\n", size_wanted);
		}
		break;
	case HTTP_EVENT_ON_DATA:
		//ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			size_readed += evt->data_len;
			//ESP_LOGI(TAG, " progress[OTA_APP_0] : %.2f%% (%d/%d)", (float)size_readed / size_wanted * 100.0, size_readed, size_wanted);
			//写flash
			err = esp_ota_write(update_handle, (const void *)evt->data, evt->data_len);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%x", err);
			}

			//ESP_LOGI(TAG, " size_readed : %d\r\n",size_readed);
			// for(int i = 0;i<evt->data_len;i++)
			// {
			// 	printf("0x%x ", ((char*)evt->data)[i]);
			// }
			// ESP_LOGI(TAG, "\r\n");
		}
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
		if (size_readed == size_wanted)
		{
			ESP_LOGI(TAG, "DONWLOAD SUCCESS!!!\r\n");
			flag_ota_init_sta = 2;
			size_readed = 0;
			size_wanted = 1;
			//OTA写结束
			if (esp_ota_end(update_handle) != ESP_OK)
			{
				ESP_LOGE(TAG, "esp_ota_end failed!");
			}
			//升级完成更新OTA data区数据，重启时根据OTA data区数据到Flash分区加载执行目标（新）固件
			err = esp_ota_set_boot_partition(update_partition);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
			}
			ESP_LOGW(TAG, "Prepare to restart system!!");
			esp_restart();
		}
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	}
	return ESP_OK;
}

void ota_start()
{
	if(flag_ota_init_sta)
	{
		flag_ota_start = 1;
	}
}

//OTA任务
void ota_service(void *pvParameter)
{
	esp_err_t err;
	//获取当前boot位置
	const esp_partition_t *configured = esp_ota_get_boot_partition();
	//获取当前系统执行的固件所在的Flash分区
	const esp_partition_t *running = esp_ota_get_running_partition();
	if (configured != running)
	{
		ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",configured->address, running->address);
		ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
	}
	ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);
	update_partition = esp_ota_get_next_update_partition(NULL);
	ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",update_partition->subtype, update_partition->address);
	assert(update_partition != NULL);

	err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
	}
	ESP_LOGI(TAG, "esp_ota_begin succeeded!!!!!!!!");
	flag_ota_init_sta = 1;

	while (1)
	{
		if(flag_ota_start)
		{
				// GET
			esp_err_t err;
			client = esp_http_client_init(&config);
			err = esp_http_client_perform(client);
			if (err == ESP_OK)
			{
			ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
					esp_http_client_get_status_code(client),
					esp_http_client_get_content_length(client));
			}
			flag_ota_start = 0;
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}
