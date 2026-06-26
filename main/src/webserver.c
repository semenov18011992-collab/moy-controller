#include "module.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>
#include "sensor_module.h"

#define TAG "WEBSERVER"

static httpd_handle_t server = NULL;

// API: получить все пины
static esp_err_t api_pins_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    char* json = sensor_export_json();
    httpd_resp_send(req, json, strlen(json));
    return ESP_OK;
}

// API: получить историю
static esp_err_t api_history_handler(httpd_req_t* req) {
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

// API: установить пин
static esp_err_t api_set_pin_handler(httpd_req_t* req) {
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

// Корень
static esp_err_t root_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    const char* msg = "{\"status\":\"OK\",\"message\":\"MOY Controller v4.0\",\"endpoints\":[\"/api/pins\",\"/api/history?pin=X&points=Y\",\"/api/pin/set?pin=X&value=Y\"]}";
    httpd_resp_send(req, msg, strlen(msg));
    return ESP_OK;
}

bool webserver_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server");
        return false;
    }
    
    httpd_uri_t uri_root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_handler
    };
    httpd_register_uri_handler(server, &uri_root);
    
    httpd_uri_t uri_api_pins = {
        .uri       = "/api/pins",
        .method    = HTTP_GET,
        .handler   = api_pins_handler
    };
    httpd_register_uri_handler(server, &uri_api_pins);
    
    httpd_uri_t uri_api_history = {
        .uri       = "/api/history",
        .method    = HTTP_GET,
        .handler   = api_history_handler
    };
    httpd_register_uri_handler(server, &uri_api_history);
    
    httpd_uri_t uri_api_set_pin = {
        .uri       = "/api/pin/set",
        .method    = HTTP_GET,
        .handler   = api_set_pin_handler
    };
    httpd_register_uri_handler(server, &uri_api_set_pin);
    
    ESP_LOGI(TAG, "Webserver started on port 80");
    return true;
}

void webserver_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Webserver stopped");
    }
}