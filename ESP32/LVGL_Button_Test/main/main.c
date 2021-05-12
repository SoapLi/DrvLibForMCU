/* LVGL Example project */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"

#include "ui_service.h"
#include "wifi_service.h"

#define TAG "LJH_DEMO"

// 主函数
void app_main()
{

	nvs_flash_init();
	wifi_init();
	// 如果要使用任务创建图形，则需要创建固定任务,否则可能会出现诸如内存损坏等问题
	xTaskCreatePinnedToCore(guiTask, "gui", 4096 * 2, NULL, 0, NULL, 1);
}
