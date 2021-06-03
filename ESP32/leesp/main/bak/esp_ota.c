#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
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


#include "esp_ota.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_app_format.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "esp_http_client.h"

#define TAG "lv_esp_ota"

static void infinite_loop(void)
{
	int i = 0;
	ESP_LOGI(TAG, "When a new firmware is available on the server, press the reset button to download it");
	while(1) {
		ESP_LOGI(TAG, "Waiting for a new firmware ... %d", ++i);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
static void http_cleanup(esp_http_client_handle_t client)
{
	esp_http_client_close(client);
	esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
	ESP_LOGE(TAG, "Exiting task due to fatal error...");
	(void)vTaskDelete(NULL);
	while (1) {
		;
	}
}

// http任务
void http_OTA_Task(void *pvParameters)
{
	esp_err_t err;
	esp_ota_handle_t update_handle = 0 ;
	const esp_partition_t *update_partition = NULL;

	/*an ota data write buffer ready to write to the flash*/
	static char ota_write_data[1025] = { 0 };



	ESP_LOGI(TAG, "Starting OTA example");
	esp_http_client_config_t config = {
		.url = "http://192.168.2.160:1257/lv_ot_demo.bin",
		//.cert_pem = (char *)server_cert_pem_start,
		.timeout_ms = 3000,
	};

	const esp_partition_t *configured = esp_ota_get_boot_partition();
	const esp_partition_t *running = esp_ota_get_running_partition();

	esp_http_client_handle_t client = esp_http_client_init(&config);
	if (client == NULL) {
		ESP_LOGE(TAG, "Failed to initialise HTTP connection");
		task_fatal_error();
	}
	err = esp_http_client_open(client, 0);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
		esp_http_client_cleanup(client);
		task_fatal_error();
	}
	update_partition = esp_ota_get_next_update_partition(NULL);
	ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",update_partition->subtype, update_partition->address);
	assert(update_partition != NULL);

	esp_http_client_fetch_headers(client);
	int binary_file_length = 0;
	/*deal with all receive packet*/
	bool image_header_was_checked = false;
	while (1) {
		int data_read = esp_http_client_read(client, ota_write_data, 1024);
		if (data_read < 0) {
			ESP_LOGE(TAG, "Error: SSL data read error");
			http_cleanup(client);
			task_fatal_error();
		} else if (data_read > 0) {
			// 通过image_header_was_checked来识别是否为第一包数据
			if (image_header_was_checked == false) {// 还没有验证更新APP的头信息
				esp_app_desc_t new_app_info;
				if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
					// check current version with downloading
					memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
					ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);
					esp_app_desc_t running_app_info;
					if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
						ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
					}

					const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
					esp_app_desc_t invalid_app_info;
					if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
						ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
					}

					// check current version with last invalid partition
					if (last_invalid_app != NULL) {
						if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
							ESP_LOGW(TAG, "New version is the same as invalid version.");
							ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
							ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
							http_cleanup(client);
							infinite_loop();
						}
					}
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
					if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
						ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
						http_cleanup(client);
						infinite_loop();
					}
#endif

					image_header_was_checked = true;

					err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
					if (err != ESP_OK) {
						ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
						http_cleanup(client);
						task_fatal_error();
					}
					ESP_LOGI(TAG, "esp_ota_begin succeeded");
				} else {// 接收的头部长度错误，小于APP头部信息长度
					ESP_LOGE(TAG, "received package is not fit len");
					http_cleanup(client);
					task_fatal_error();
				}
			}
			err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
			if (err != ESP_OK) {
				http_cleanup(client);
				task_fatal_error();
			}
			binary_file_length += data_read;
			ESP_LOGD(TAG, "Written image length %d", binary_file_length);
		} else if (data_read == 0) {// 读取完成，
		/*
			* As esp_http_client_read never returns negative error code, we rely on
			* `errno` to check for underlying transport connectivity closure if any
			*/
			if (errno == ECONNRESET || errno == ENOTCONN) {
				ESP_LOGE(TAG, "Connection closed, errno = %d", errno);
				break;
			}
			if (esp_http_client_is_complete_data_received(client) == true) {
				ESP_LOGI(TAG, "Connection closed");
				break;
			}
		}
	}
	ESP_LOGI(TAG, "Total Write binary data length: %d", binary_file_length);
	if (esp_http_client_is_complete_data_received(client) != true) {
		ESP_LOGE(TAG, "Error in receiving complete file");
		http_cleanup(client);
		task_fatal_error();
	}

	err = esp_ota_end(update_handle);
	if (err != ESP_OK) {
		if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
			ESP_LOGE(TAG, "Image validation failed, image is corrupted");
		}
		ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
		http_cleanup(client);
		task_fatal_error();
	}

	err = esp_ota_set_boot_partition(update_partition);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
		http_cleanup(client);
		task_fatal_error();
	}
	ESP_LOGI(TAG, "Prepare to restart system!");
	esp_restart();
	return ;
}



















