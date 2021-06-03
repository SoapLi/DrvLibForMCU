
#ifndef WEATHER_HTTP_SERVICE_H
#define WEATHER_HTTP_SERVICE_H

//天气解析结构体
typedef struct 
{
	char cit[20];
	char weather_text[20];
	char weather_code[2];
	char temperatur[3];
	char time_last_update[30];
}weather_info;

void http_weather_task(void *pvParameters);

void get_weather_info(weather_info *info);

#endif/* __USER_HTTPS_H__ */
