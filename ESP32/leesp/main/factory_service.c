#include "factory_service.h"
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

#define TAG "LJH_FACTORY_SERVICE"



void goto_factory()
{
	esp_err_t err;
	err = esp_ota_set_boot_partition(esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL));
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
	}
	ESP_LOGW(TAG, "Prepare to restart system!!");
	esp_restart();

}
	
