
#ifndef SNTP_SERVICE_H
#define SNTP_SERVICE_H


//天气解析结构体
typedef struct 
{
	char real_time[64];

}sntp_info;

void esp_initialize_sntp(void);
void esp_wait_sntp_sync(void);
void esp_get_local_time(sntp_info * sntp);

#endif/* __USER_HTTPS_H__ */
