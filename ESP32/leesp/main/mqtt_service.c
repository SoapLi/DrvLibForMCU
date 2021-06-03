

#include "mqtt_service.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "../components/cus_esp_mqtt/esp-mqtt/include/mqtt_client.h"

#define TAG "LJH_MQTT"



// MQTT事件
static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;	// 连接标志位

// MQTT客户端句柄
esp_mqtt_client_handle_t client;

// MQTT事件
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	esp_mqtt_client_handle_t client = event->client;
	int msg_id;
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:// MQTT连接成功
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
			//发送订阅
			msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 1);
			ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
			//取消订阅
			// msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
			// ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
			break;
		case MQTT_EVENT_DISCONNECTED:// MQTT断开连接事件
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			//mqtt连上事件
			xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
			break;

		case MQTT_EVENT_SUBSCRIBED:// MQTT发送订阅事件
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "订阅成功", 0, 0, 0);
			ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:// MQTT取消订阅事件
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:// MQTT发布事件
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:// MQTT接受数据事件
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);   //主题
			printf("DATA=%.*s\r\n", event->data_len, event->data);      //内容
			break;
		case MQTT_EVENT_ERROR:// MQTT错误事件
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
			break;
		case MQTT_EVENT_ANY:
			break;
		case MQTT_EVENT_BEFORE_CONNECT:
			break;			
	}
	return ESP_OK;
}

//mqtt初始化
static void mqtt_app_start(void)
{
	mqtt_event_group = xEventGroupCreate();
	esp_mqtt_client_config_t mqtt_cfg = {
		.host = "www.passingworld.vip",//"192.168.2.104",            //MQTT服务器IP
		.event_handle = mqtt_event_handler, //MQTT事件
		.port = 1883,                         //端口
		.username = "TaobaoTestUser",                //用户名
		.password = "aa12345678",               //密码
		// .user_context = (void *)your_context
	};

#if CONFIG_BROKER_URL_FROM_STDIN
	char line[128];
	if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
		int count = 0;
		printf("Please enter url of mqtt broker\n");
		while (count < 128) {
			int c = fgetc(stdin);
			if (c == '\n') {
				line[count] = '\0';
				break;
			} else if (c > 0 && c < 127) {
				line[count] = c;
				++count;
			}
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		mqtt_cfg.uri = line;
		printf("Broker url: %s\n", line);
	} else {
		ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
		abort();
	}
#endif /* CONFIG_BROKER_URL_FROM_STDIN */
	client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(client);
	//等mqtt连上
	xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void mqtt_task(void *pvParameter)
{
	mqtt_app_start();
	while (1) {
		esp_mqtt_client_publish(client, "/topic/qos0", "Hello MQTT ,I am HongXu", 0, 0, 0);
		ESP_LOGI(TAG, "mqtt_task");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}