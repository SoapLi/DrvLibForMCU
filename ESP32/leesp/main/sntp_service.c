#include "sntp_service.h"
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
#include "driver/gpio.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"
#include "nvs_flash.h"
#include "esp_wpa2.h"

#define TAG "LJH_SNTP"


void esp_initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_init();
}

void esp_wait_sntp_sync()
{
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    while (timeinfo.tm_year < (2019 - 1900)) {
        //ESP_LOGD(TAG, "Waiting for system time to be set... (%d)", ++retry);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
}

void esp_get_local_time(sntp_info * sntp)
{
    time_t now = 0;
    struct tm timeinfo = { 0 };
	time(&now);
	localtime_r(&now, &timeinfo);
    strftime(sntp->real_time,sizeof(sntp->real_time), "%c", &timeinfo);
}
