#include "weather_http_service.h"
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
#include "nvs_flash.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "esp_wpa2.h"
#include "os.h"

#define TAG "LJH_HTTP"


//http组包宏，获取天气的http接口参数
#define WEB_SERVER          "api.thinkpage.cn"              
#define WEB_PORT            "80"
#define WEB_URL             "/v3/weather/now.json?key="
#define host 		        "api.thinkpage.cn"
#define APIKEY		        "g3egns3yk2ahzb0p"       
#define city		        "xian"
#define language	        "en"
/*
===========================
全局变量定义
=========================== 
*/
//http请求包
static const char *REQUEST = "GET "WEB_URL""APIKEY"&location="city"&language="language" HTTP/1.1\r\n"
    "Host: "WEB_SERVER"\r\n"
    "Connection: close\r\n"
    "\r\n";

weather_info weathe = {0};

void get_weather_info(weather_info *info)
{
	os_memcpy(info,&weathe,sizeof(weathe));	
}

// 解析json数据 只处理 解析 城市 天气 天气代码  温度  其他的自行扩展
void cjson_to_struct_info(char *text)
{
cJSON *root, *psub;
	cJSON *arrayItem;
	//截取有效json
	char *index = strchr(text, '{');
	strcpy(text, index);
	root = cJSON_Parse(text);
	if (root != NULL) {
		psub = cJSON_GetObjectItem(root, "results");
		arrayItem = cJSON_GetArrayItem(psub, 0);

		cJSON *locat = cJSON_GetObjectItem(arrayItem, "location");
		cJSON *now = cJSON_GetObjectItem(arrayItem, "now");
		cJSON *last_update = cJSON_GetObjectItem(arrayItem, "last_update");

		if ((locat != NULL)&&(now != NULL)&&(last_update !=NULL))
		{
			psub = cJSON_GetObjectItem(locat, "name");
			sprintf(weathe.cit, "%s", psub->valuestring);
			ESP_LOGI(TAG, "city:%s", weathe.cit);

			psub = cJSON_GetObjectItem(now, "text");
			sprintf(weathe.weather_text, "%s", psub->valuestring);
			ESP_LOGI(TAG, "weather:%s", weathe.weather_text);
            
			psub = cJSON_GetObjectItem(now, "code");
			sprintf(weathe.weather_code, "%s", psub->valuestring);
			//ESP_LOGI(HTTP_TAG,"%s",weathe.weather_code);

			psub = cJSON_GetObjectItem(now, "temperature");
			sprintf(weathe.temperatur, "%s", psub->valuestring);
			ESP_LOGI(TAG, "temperatur:%s", weathe.temperatur);

			sprintf(weathe.time_last_update, "%s", last_update->valuestring);
			ESP_LOGI(TAG, "time_last_update: %s", weathe.time_last_update);
		}
	}
	cJSON_Delete(root);
}
// http任务
void http_weather_task(void *pvParameters)
{
	const struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct addrinfo *res;
	struct in_addr *addr;
	int s, r;
	char recv_buf[1024];
	char mid_buf[1024];
	int index;
	while(1) {
		//DNS域名解析
		int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
		if(err != 0 || res == NULL) {
			ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p\r\n", err, res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}

		//打印获取的IP
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
		//ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

		//新建socket
		s = socket(res->ai_family, res->ai_socktype, 0);
		if(s < 0) {
			ESP_LOGE(TAG, "... Failed to allocate socket.\r\n");
			close(s);
			freeaddrinfo(res);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			continue;
		}

		//连接ip
		if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
			ESP_LOGE(TAG, "... socket connect failed errno=%d\r\n", errno);
			close(s);
			freeaddrinfo(res);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		freeaddrinfo(res);

		//发送http包
		if (write(s, REQUEST, strlen(REQUEST)) < 0) {
			ESP_LOGE(TAG, "... socket send failed\r\n");
			close(s);
			vTaskDelay(4000 / portTICK_PERIOD_MS);
			continue;
		}
		ESP_LOGI(TAG, "http req is : %s\r\n", REQUEST);

		//清缓存
		memset(mid_buf,0,sizeof(mid_buf));
		//获取http应答包
		do {
			bzero(recv_buf, sizeof(recv_buf));
			r = read(s, recv_buf, sizeof(recv_buf)-1);
			strcat(mid_buf,recv_buf);
		} while(r > 0);
		//ESP_LOGE(TAG, "Rev:%s\n",mid_buf);
		//json解析
		cjson_to_struct_info(mid_buf);
		//关闭socket，http是短连接
		close(s);

		//延时一会
		for(int countdown = 10; countdown >= 0; countdown--) {
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
	}
}