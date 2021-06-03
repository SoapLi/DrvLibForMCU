#include "ui_boot_service.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event_loop.h"

#include "lvgl.h"
#include "lvgl_helpers.h"
#include "../components/lv_examples/lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

#include "weather_http_service.h"
#include "wifi_service.h"
#include "sntp_service.h"
#include "adc_service.h"
#include "ota_service.h"
#include "factory_service.h"

#define TAG "LJH_BOOT_UI"

#define LV_TICK_PERIOD_MS 2

lv_obj_t *btn_check_update,*btn_factory_mode;
lv_obj_t *label_ota_sta,*label_version;
lv_obj_t* img_start_logo;

SemaphoreHandle_t xGuiSemaphore;	 // 创建一个GUI信号量
static void lv_tick_task(void *arg); // LVGL 时钟任务

LV_IMG_DECLARE(boot_logo)

// LVGL 时钟任务
static void lv_tick_task(void *arg)
{
	(void)arg;
	static int cnt = 0;
	cnt++;
	if(cnt == 300)
	{
		if(get_ota_init_sta() == 1 && get_wifi_sta() == 1)
		{
			lv_label_set_text(label_ota_sta, "OTA ready!");
			
		}else if (get_ota_init_sta() == 2)
		{
			lv_label_set_text(label_ota_sta, "reboot...");
		}
		else{
			lv_label_set_text(label_ota_sta, "OTA");
		}
		cnt = 0;
	 }
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void btn_check_update_event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_PRESSED)
	{
		ESP_LOGI(TAG, "btn_check_update_event_handler -> Clicked pressed\n"); // 按下
	}
	else if (event == LV_EVENT_RELEASED)
	{
		ESP_LOGI(TAG, "btn_check_update_event_handler -> Clicked released\n"); // 释放
		if(get_ota_init_sta())
		{
			ota_start();
		}
		else{
			
		}
	}
}

// DropDownList 控件事件处理函数
static void DropDownList_event_handler(lv_obj_t * obj, lv_event_t event)
{
	if(event == LV_EVENT_VALUE_CHANGED) {
		char buf[32];
		lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
		ESP_LOGI(TAG,"Option: %s\n", buf);
		set_ota_bin_path(buf);
	}
}

// bar任务
static void bar_update_anim(lv_task_t * t)
{
	static uint32_t x = 0;				// 静态变量Bar可变值
	lv_obj_t * bar = t->user_data;		// 从任务参数中获取bar对象
	static char buf[50];
	int  readed = get_size_readed();
	int  wanted = get_size_wanted();
	lv_snprintf(buf, sizeof(buf), "Update:%d/%d",readed, wanted);// 创建字符串
	lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);// 显示字符串
	//ESP_LOGI(TAG, " %d / %d",	readed,wanted); // 按下
	//ESP_LOGI(TAG, " %d",	(int)        (((float)readed/wanted)*100 )            ); // 按下
	//lv_bar_set_range(bar,0,wanted);	
	lv_bar_set_value(bar, (int)        (((float)readed/wanted)*100 )   , LV_ANIM_OFF);				// 设置值，关闭动画
	lv_obj_align(bar, NULL,LV_ALIGN_IN_BOTTOM_LEFT, 15, -30);// 重新设置对齐
}

void guiBootTask(void *pvParameter)
{
	(void)pvParameter;
	xGuiSemaphore = xSemaphoreCreateMutex(); // 创建GUI信号量
	lv_init();								 // 初始化LittlevGL
	lvgl_driver_init();						 // 初始化液晶SPI驱动 触摸芯片SPI/IIC驱动

	// 初始化缓存
	static lv_color_t buf1[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	uint32_t size_in_px = DISP_BUF_SIZE;
	lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

	// 添加并注册触摸驱动
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	// 添加并注册触摸驱动
	ESP_LOGI(TAG, "Add Register Touch Drv");
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.read_cb = touch_driver_read;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_drv_register(&indev_drv);

	// 定期处理GUI回调
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

	btn_check_update = lv_btn_create(lv_scr_act(), NULL);		   // 在主屏幕上创建一个按钮
	lv_obj_set_event_cb(btn_check_update, btn_check_update_event_handler);		   // 为按钮控件添加事件处理
	lv_obj_set_width(btn_check_update, 120);
	lv_obj_align(btn_check_update, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -5, -10); //
	lv_obj_t *label_btnSetting = lv_label_create(btn_check_update, NULL); // 在按钮控件上创建一个标签
	lv_label_set_text(label_btnSetting, "check update");			// 设置标签名称为Button

	label_ota_sta = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label_ota_sta, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label_ota_sta, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label_ota_sta, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	lv_label_set_text(label_ota_sta, "OTA");
	lv_obj_set_width(label_ota_sta, 150);							 // 设置标签宽度
	lv_obj_align(label_ota_sta, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0); // 对齐到中心偏上

	label_version = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label_version, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label_version, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label_version, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	lv_label_set_text(label_version, "FACTORY V0.1");
	lv_obj_set_width(label_version, 150);							 // 设置标签宽度
	lv_obj_align(label_version, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 20); // 对齐到中心偏上

	img_start_logo = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img_start_logo, &boot_logo);
	lv_obj_align(img_start_logo, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

	lv_obj_t * ddlist1 = lv_dropdown_create(lv_scr_act(), NULL);		// 在屏幕中心创建一个下拉控件
	lv_dropdown_set_options(ddlist1,
			"/ota_1.bin\n"							// 设置下拉内容，第一条为初始化显示的内容
			"/ota_2.bin\n"
			"/ota_3.bin\n");
	lv_obj_align(ddlist1, NULL, LV_ALIGN_CENTER, 90, -5);			// 对齐到屏幕左上角
	lv_obj_set_event_cb(ddlist1, DropDownList_event_handler);			// 注册事件处理函数

	lv_obj_t * bar_update = lv_bar_create(lv_scr_act(), NULL);
	lv_obj_set_width(bar_update, 160);
	lv_obj_set_style_local_value_font(bar_update, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_theme_get_font_small());
	lv_obj_set_style_local_value_align(bar_update, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
	lv_obj_set_style_local_value_ofs_y(bar_update, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
	lv_obj_set_style_local_margin_bottom(bar_update, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 3);
	lv_obj_align(bar_update, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 15, -30);
	lv_task_create(bar_update_anim, 50, LV_TASK_PRIO_MID, bar_update);

	while (1)
	{
		vTaskDelay(2);
		// 尝试锁定信号量，如果成功，调用处理LVGL任务
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE)
		{
			lv_task_handler();			   // 处理LVGL任务
			xSemaphoreGive(xGuiSemaphore); // 释放信号量
		}
	}
	vTaskDelete(NULL); // 删除任务
}