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

#include "lvgl.h"
#include "lvgl_helpers.h"
#include "../components/lv_examples/lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

#include "weather_http_service.h"
#include "wifi_service.h"
#include "sntp_service.h"
#include "adc_service.h"
#include "ota_service.h"
#include "factory_service.h"

#define TAG "LJH_UI"

#define LV_TICK_PERIOD_MS 2

LV_IMG_DECLARE(harmonyos_logo)

lv_obj_t *btn_factory_mode,*img_start_logo;

SemaphoreHandle_t xGuiSemaphore;	 // 创建一个GUI信号量
static void lv_tick_task(void *arg); // LVGL 时钟任务

LV_IMG_DECLARE(logo1)

// LVGL 时钟任务
static void lv_tick_task(void *arg)
{
	(void)arg;
	static int cnt = 0;
	cnt++;
	if(cnt == 300)
	{
		cnt = 0;
	 }
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void btn_close_event_handler(lv_obj_t *obj, lv_event_t event)
{
	
	if (event == LV_EVENT_PRESSED)
	{
		goto_factory();
	}
}


static void btn_factory_event_handler(lv_obj_t *obj, lv_event_t event)
{
	if (event == LV_EVENT_PRESSED)
	{
		ESP_LOGI(TAG, "btn_factory_event_handler -> Clicked pressed\n"); // 按下
	}
	else if (event == LV_EVENT_RELEASED)
	{
		ESP_LOGI(TAG, "btn_factory_event_handler -> Clicked pressed\n"); // 按下
		

		lv_obj_t * win = lv_win_create(lv_scr_act(), NULL);				// 在主屏幕上创建一个窗口
		lv_win_set_title(win, "Welcome");					// 设置窗口标题
		lv_win_set_btn_width(win,0);									// 设置窗口按钮的宽度，0为正方形
		lv_obj_set_size(win, 280, 170);									// 设置窗口大小
		lv_obj_align(win, NULL, LV_ALIGN_CENTER, 0, 0);					// 设置对齐到中心
		lv_win_set_header_height(win, 25);								// 设置窗口标题栏高度
		lv_win_add_btn_left(win,LV_SYMBOL_SETTINGS);					// 在窗口左边添加一个设置图标

		static lv_style_t style;										// 给设置窗口添加一个阴影风格
		lv_style_init(&style);											// 初始化风格
		lv_style_set_shadow_width(&style,LV_STATE_DEFAULT,20);			// 添加阴影宽度
		lv_style_set_shadow_opa(&style,LV_STATE_DEFAULT,LV_OPA_70);		// 阴影透明度		
		lv_style_set_shadow_color(&style,LV_STATE_DEFAULT,LV_COLOR_BLUE);// 阴影颜色

		lv_obj_add_style(win, LV_BTN_PART_MAIN, &style);


		lv_obj_t * close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE);	// 添加一个关闭并使用win内置的关闭回调操作
		lv_obj_set_event_cb(close_btn, btn_close_event_handler);			// 设置按钮事件调回函数 lv_win_close_event_cb是内部回调不用定义数

		lv_obj_t * txt = lv_label_create(win, NULL);					// 在窗口控件中创建一个标签
		lv_label_set_text(txt, "HarmonyOS is a brand-new\n"
								"distributed operating system.\n"
								"system for all scenarios.\n"
								"HarmonyOS 1.0 have verified\n"
								"the possibility of the\n"
								"distributed technology");
















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


	img_start_logo = lv_img_create(lv_scr_act(), NULL);
	lv_img_set_src(img_start_logo, &harmonyos_logo);
	lv_obj_align(img_start_logo, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);


	btn_factory_mode = lv_btn_create(lv_scr_act(), NULL);		   // 在主屏幕上创建一个按钮
	lv_obj_set_event_cb(btn_factory_mode, btn_factory_event_handler);		   // 为按钮控件添加事件处理
	lv_obj_set_width(btn_factory_mode, 60);
	lv_obj_align(btn_factory_mode, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -5, -20); //
	lv_obj_t *label_factory = lv_label_create(btn_factory_mode, NULL); // 在按钮控件上创建一个标签
	lv_label_set_text(label_factory, "next");			// 设置标签名称为Button

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