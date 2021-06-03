
#include <string.h>

#include "get_weather.h"
#include "esp_log.h"
#include "cJSON.h"
#include "cJSON_Utils.h"

#define TAG "lv_esp_ota"


// 解析json数据 只处理 解析 城市 天气 天气代码  温度  其他的自行扩展
void cjson_to_struct_info(char *text)
{
	cJSON *root,*psub,*Item;
	//截取有效json
	char *index=strchr(text,'{');
	strcpy(text,index);
	root = cJSON_Parse(text);
	if(root!=NULL){
		psub = cJSON_GetObjectItem(root, "weatherinfo");
		if(psub!=NULL){
			Item=cJSON_GetObjectItem(psub,"cityid");
			if(Item!=NULL){
				ESP_LOGI(TAG,"cityid:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"city");
			if(Item!=NULL){
				ESP_LOGI(TAG,"城市:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"WD");
			if(Item!=NULL){
				ESP_LOGI(TAG,"风向:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"temp");
			if(Item!=NULL){
				ESP_LOGI(TAG,"温度:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"SD");
			if(Item!=NULL){
				ESP_LOGI(TAG,"湿度:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"AP");
			if(Item!=NULL){
				ESP_LOGI(TAG,"气压:%s\n",Item->valuestring);
			}

			Item=cJSON_GetObjectItem(psub,"time");
			if(Item!=NULL){
				ESP_LOGI(TAG,"时间:%s\n",Item->valuestring);
			}
		}
	}
	cJSON_Delete(root);
}