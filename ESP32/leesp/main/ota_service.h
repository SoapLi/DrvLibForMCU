
#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

void ota_service(void *pvParameter);
void ota_start();
int get_size_wanted(void);
int get_size_readed(void);
int get_ota_init_sta(void);
void set_ota_bin_path(char* path);

#endif/* __USER_HTTPS_H__ */
