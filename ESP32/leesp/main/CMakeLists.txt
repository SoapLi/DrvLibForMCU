set(SOURCES main.c wifi_service.c mqtt_service.c mbs_service.c ui_service.c ui_boot_service.c modbus_params.c weather_http_service.c sntp_service.c logo1.c logo2.c
ota_service.c oled.c fonts.c adc_service.c factory_service.c boot_logo.c harmonyos_logo.c)
set(include_dirs "."
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/esp_adc_cal/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/app_update/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/spi_flash/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/nvs_flash/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/bootloader_support/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/json/cJSON"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/esp_http_client/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/nghttp/nghttp2/third-party/http-parser"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/wpa_supplicant/include/esp_supplicant"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/wpa_supplicant/port/include"

)
idf_component_register(SRCS ${SOURCES}
					INCLUDE_DIRS . 
					"${include_dirs}"
					REQUIRES lvgl_esp32_drivers lvgl_touch lvgl_tft lvgl lv_examples)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
