#include "webserver_priv.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "esp_system.h"
#include "hal/efuse_hal.h"
#include "sensor_module.h"
#include "config_manager.h"
#include "pin_config.h"
#include <string.h>

#define TAG "WEBSERVER_API"

// ============================================
// ГЛОБАЛЬНЫЙ БУФЕР
// ============================================
char http_response_buffer[MAX_HTTP_RESPONSE_SIZE];

// ============================================
// ОБРАБОТЧИК /api/pins
// ============================================
esp_err_t api_pins_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_json();
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

// ============================================
// ОБРАБОТЧИК /api/history
// ============================================
esp_err_t api_history_handler(httpd_req_t* req) {
    char query[64] = {0};
    uint8_t pin_id = 34;
    int points = 50;
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[16];
        if (httpd_query_key_value(query, "pin", param, sizeof(param)) == ESP_OK) {
            pin_id = atoi(param);
        }
        if (httpd_query_key_value(query, "points", param, sizeof(param)) == ESP_OK) {
            points = atoi(param);
            if (points > 100) points = 100;
            if (points < 10) points = 10;
        }
    }
    
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_history_json(pin_id, points);
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

// ============================================
// ОБРАБОТЧИК /api/pin/set
// ============================================
esp_err_t api_set_pin_handler(httpd_req_t* req) {
    char query[64] = {0};
    uint8_t pin_id = 0;
    float value = 0;
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[16];
        if (httpd_query_key_value(query, "pin", param, sizeof(param)) == ESP_OK) {
            pin_id = atoi(param);
        }
        if (httpd_query_key_value(query, "value", param, sizeof(param)) == ESP_OK) {
            value = atof(param);
        }
    }
    
    char* response = sensor_set_pin(pin_id, value);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}