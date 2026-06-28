#pragma once
#include "esp_http_server.h"
#include "cJSON.h"
#include "pin_config.h"          // ← ДОБАВИТЬ
#include "config_manager.h"      // ← ДОБАВИТЬ

#ifdef __cplusplus
extern "C" {
#endif

// === БУФЕР ===
#define MAX_HTTP_RESPONSE_SIZE 4096   // ← ДОБАВИТЬ

// === ВНЕШНИЕ ФУНКЦИИ (из sensor_module) ===
extern char* sensor_export_json(void);
extern char* sensor_export_history_json(uint8_t pin, int points);
extern char* sensor_set_pin(uint8_t pin, float value);

// === ОБРАБОТЧИКИ ===
esp_err_t root_handler(httpd_req_t *req);
esp_err_t api_pins_handler(httpd_req_t *req);
esp_err_t api_history_handler(httpd_req_t *req);
esp_err_t api_set_pin_handler(httpd_req_t *req);
esp_err_t pin_config_handler(httpd_req_t *req);
esp_err_t pins_config_all_handler(httpd_req_t *req);
esp_err_t pin_delete_handler(httpd_req_t *req);
esp_err_t system_info_handler(httpd_req_t *req);

// === РЕГИСТРАЦИЯ ===
void register_handlers(httpd_handle_t server);

// === БУФЕР ===
extern char http_response_buffer[MAX_HTTP_RESPONSE_SIZE];

#ifdef __cplusplus
}
#endif