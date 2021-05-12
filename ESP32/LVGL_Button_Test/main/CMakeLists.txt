set(SOURCES main.c weather_https_service.c ui_service.c wifi_service.c weather_http_service.c)
set(include_dirs "."
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/spi_flash/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/nvs_flash/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/wpa_supplicant/include/esp_supplicant"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/json/cJSON"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/mbedtls/mbedtls/include"
"C:/ESP32/ESP_IDF/esp-idf-v4.2/components/wpa_supplicant/port/include"


)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS .
					"${include_dirs}"
                    REQUIRES lvgl_esp32_drivers lvgl_touch lvgl_tft lvgl lv_examples)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
