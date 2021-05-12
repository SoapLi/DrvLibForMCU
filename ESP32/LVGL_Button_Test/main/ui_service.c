#include "ui_service.h"
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
// Littlevgl 头文件
#include "lvgl/lvgl.h"	  // LVGL头文件
#include "lvgl_helpers.h" // 助手 硬件驱动相关
#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

#include "weather_https_service.h"

#define TAG "LJH_DEMO"

#define LV_TICK_PERIOD_MS 1

lv_obj_t *label4;
lv_obj_t *label5;
lv_obj_t *label6;
lv_obj_t *label7;
lv_obj_t *label8;

SemaphoreHandle_t xGuiSemaphore;	 // 创建一个GUI信号量
static void lv_tick_task(void *arg); // LVGL 时钟任务

// LVGL 时钟任务
static void lv_tick_task(void *arg)
{
	(void)arg;
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

// Button1事件 正常按钮
static void button1_event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_PRESSED)
	{
		ESP_LOGI(TAG, "button1_event_handler->Clicked pressed\n"); // 按下

		https_request_by_GET(HTTPS_URL_XA);
	}
	else if (event == LV_EVENT_RELEASED)
	{
		ESP_LOGI(TAG, "button1_event_handler->Clicked released\n"); // 释放
	}
}

void guiTask(void *pvParameter)
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

	///////////////////////////////////////////////////
	///////////////普通按钮 Button1/////////////////////////////
	///////////////////////////////////////////////////
	lv_obj_t *btn1 = lv_btn_create(lv_scr_act(), NULL);		   // 在主屏幕上创建一个按钮
	lv_obj_set_event_cb(btn1, button1_event_handler);		   // 为按钮控件添加事件处理
	lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 170); // 对齐到中心，X偏移70 Y偏移-80

	lv_obj_t *label_btn1 = lv_label_create(btn1, NULL); // 在按钮控件上创建一个标签
	lv_label_set_text(label_btn1, "Setting");			// 设置标签名称为Button

	lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL); // 在主屏幕创建一个标签
	lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);	// 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label1, true);						// 使能字符命令重新对字符上色
	lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);		// 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label1, "City: ");
	lv_obj_set_width(label1, 150);							// 设置标签宽度
	lv_obj_align(label1, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0); // 对齐到中心偏上

	lv_obj_t *label2 = lv_label_create(lv_scr_act(), NULL); // 在主屏幕创建一个标签
	lv_label_set_long_mode(label2, LV_LABEL_LONG_BREAK);	// 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label2, true);						// 使能字符命令重新对字符上色
	lv_label_set_align(label2, LV_LABEL_ALIGN_CENTER);		// 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label2, "Weather: ");
	lv_obj_set_width(label2, 150);							 // 设置标签宽度
	lv_obj_align(label2, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 40); // 对齐到中心偏上

	lv_obj_t *label3 = lv_label_create(lv_scr_act(), NULL); // 在主屏幕创建一个标签
	lv_label_set_long_mode(label3, LV_LABEL_LONG_BREAK);	// 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label3, true);						// 使能字符命令重新对字符上色
	lv_label_set_align(label3, LV_LABEL_ALIGN_CENTER);		// 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label3, "Temperature: ");
	lv_obj_set_width(label3, 150);							 // 设置标签宽度
	lv_obj_align(label3, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 80); // 对齐到中心偏上

	lv_obj_t *label7 = lv_label_create(lv_scr_act(), NULL); // 在主屏幕创建一个标签
	lv_label_set_long_mode(label7, LV_LABEL_LONG_BREAK);	// 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label7, true);						// 使能字符命令重新对字符上色
	lv_label_set_align(label7, LV_LABEL_ALIGN_CENTER);		// 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label7, "last update time: ");
	lv_obj_set_width(label7, 150);							  // 设置标签宽度
	lv_obj_align(label7, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 120); // 对齐到中心偏上

	label4 = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label4, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label4, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label4, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label4, "null");
	lv_obj_set_width(label4, 150);							 // 设置标签宽度
	lv_obj_align(label4, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0); // 对齐到中心偏上

	label5 = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label5, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label5, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label5, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label5, "null");
	lv_obj_set_width(label5, 150);							  // 设置标签宽度
	lv_obj_align(label5, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 40); // 对齐到中心偏上

	label6 = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label6, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label6, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label6, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label6, "null");
	lv_obj_set_width(label6, 150);							  // 设置标签宽度
	lv_obj_align(label6, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 80); // 对齐到中心偏上

	label8 = lv_label_create(lv_scr_act(), NULL);		 // 在主屏幕创建一个标签
	lv_label_set_long_mode(label8, LV_LABEL_LONG_BREAK); // 标签长内容框，保持控件宽度，内容过长就换行
	lv_label_set_recolor(label8, true);					 // 使能字符命令重新对字符上色
	lv_label_set_align(label8, LV_LABEL_ALIGN_CENTER);	 // 内容居中对齐
	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label8, "null");
	lv_obj_set_width(label8, 150);							   // 设置标签宽度
	lv_obj_align(label8, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 120); // 对齐到中心偏上

	while (1)
	{
		vTaskDelay(10);
		// 尝试锁定信号量，如果成功，调用处理LVGL任务
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE)
		{
			lv_task_handler();			   // 处理LVGL任务
			xSemaphoreGive(xGuiSemaphore); // 释放信号量
		}
	}
	vTaskDelete(NULL); // 删除任务
}