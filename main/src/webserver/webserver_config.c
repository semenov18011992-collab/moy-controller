#include "webserver_priv.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "config_manager.h"
#include "pin_config.h"
#include "esp_system.h"
#include "esp_chip_info.h"      // ← ДОБАВИТЬ
#include <string.h>

#define TAG "WEBSERVER_CONFIG"

// ============================================
// УДАЛИТЬ ВСЕ ЭТИ ФУНКЦИИ (они уже есть в pin_config.c):
// - pin_type_to_string
// - pin_mode_to_string
// - pin_string_to_type
// - pin_string_to_mode
// ============================================

// ============================================
// ОБРАБОТЧИК /api/pin/config (POST)
// ============================================
esp_err_t pin_config_handler(httpd_req_t *req) {
    char content[512];
    size_t recv_size = req->content_len;
    
    if (recv_size > sizeof(content) - 1) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_size] = '\0';
    
    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    
    cJSON *pin_json = cJSON_GetObjectItem(json, "pin");
    cJSON *name_json = cJSON_GetObjectItem(json, "name");
    cJSON *type_json = cJSON_GetObjectItem(json, "type");
    cJSON *mode_json = cJSON_GetObjectItem(json, "mode");
    cJSON *enabled_json = cJSON_GetObjectItem(json, "enabled");
    
    if (!pin_json || !cJSON_IsNumber(pin_json)) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing pin number");
        return ESP_FAIL;
    }
    
    uint8_t pin = (uint8_t)pin_json->valueint;
    if (pin >= MAX_PINS) {
        cJSON_Delete(json);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid pin number");
        return ESP_FAIL;
    }
    
    pin_config_t cfg = {
        .pin = pin,
        .enabled = enabled_json && cJSON_IsBool(enabled_json) ? 
                   (enabled_json->valueint ? 1 : 0) : 1,
        .pullup = 0,
        .inverted = 0,
        .min_val = 0,
        .max_val = 100,
        .crc = 0
    };
    
    if (name_json && cJSON_IsString(name_json)) {
        strncpy(cfg.name, name_json->valuestring, MAX_PIN_NAME_LEN - 1);
        cfg.name[MAX_PIN_NAME_LEN - 1] = '\0';
    } else {
        snprintf(cfg.name, MAX_PIN_NAME_LEN, "GPIO_%d", pin);
    }
    
    if (type_json && cJSON_IsString(type_json)) {
        cfg.type = pin_string_to_type(type_json->valuestring);
    } else {
        cfg.type = PIN_TYPE_SENSOR;
    }
    
    if (mode_json && cJSON_IsString(mode_json)) {
        cfg.mode = pin_string_to_mode(mode_json->valuestring);
    } else {
        cfg.mode = PIN_MODE_DIGITAL;
    }
    
    esp_err_t err = config_save_pin(&cfg);
    cJSON_Delete(json);
    
    if (err != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save config");
        return ESP_FAIL;
    }
    
    config_apply_pin(&cfg);
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", true);
    cJSON_AddStringToObject(response, "message", "Config saved");
    cJSON_AddNumberToObject(response, "pin", pin);
    
    char *response_str = cJSON_PrintUnformatted(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

// ============================================
// ОБРАБОТЧИК /api/pins/config/all
// ============================================
esp_err_t pins_config_all_handler(httpd_req_t *req) {
    pin_config_t configs[MAX_PINS];
    uint8_t count = 0;
    char *response = http_response_buffer;
    
    memset(response, 0, MAX_HTTP_RESPONSE_SIZE);
    strcpy(response, "{\"configs\":[");
    
    esp_err_t err = config_load_all(configs, &count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "config_load_all failed: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load configs");
        return ESP_FAIL;
    }
    
    for (int i = 0; i < count && i < MAX_PINS; i++) {
        char temp[256];
        const char *type_str = pin_type_to_string(configs[i].type);
        const char *mode_str = pin_mode_to_string(configs[i].mode);
        
        snprintf(temp, sizeof(temp),
            "{\"pin\":%d,\"name\":\"%s\",\"type\":\"%s\",\"mode\":\"%s\",\"enabled\":%s}",
            configs[i].pin,
            configs[i].name,
            type_str,
            mode_str,
            configs[i].enabled ? "true" : "false"
        );
        
        if (i > 0) {
            strncat(response, ",", MAX_HTTP_RESPONSE_SIZE - strlen(response) - 1);
        }
        strncat(response, temp, MAX_HTTP_RESPONSE_SIZE - strlen(response) - 1);
    }
    
    strncat(response, "]}", MAX_HTTP_RESPONSE_SIZE - strlen(response) - 1);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// ============================================
// ОБРАБОТЧИК /api/pin/delete
// ============================================
esp_err_t pin_delete_handler(httpd_req_t *req) {
    char query[64];
    char pin_str[8] = {0};
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "pin", pin_str, sizeof(pin_str));
    }
    
    if (strlen(pin_str) == 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing pin parameter");
        return ESP_FAIL;
    }
    
    uint8_t pin = atoi(pin_str);
    if (pin >= MAX_PINS) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid pin number");
        return ESP_FAIL;
    }
    
    esp_err_t err = config_delete_pin(pin);
    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to delete config");
        return ESP_FAIL;
    }
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddBoolToObject(response, "success", true);
    cJSON_AddStringToObject(response, "message", "Config deleted");
    cJSON_AddNumberToObject(response, "pin", pin);
    
    char *response_str = cJSON_PrintUnformatted(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}

// ============================================
// ИСПРАВЛЕННЫЙ ОБРАБОТЧИК /api/system
// ============================================
esp_err_t system_info_handler(httpd_req_t *req) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "free_heap", esp_get_free_heap_size());
    cJSON_AddNumberToObject(response, "min_free_heap", esp_get_minimum_free_heap_size());
    
    // Используем правильную функцию для получения информации о чипе
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(response, "chip_model", "ESP32");
    cJSON_AddNumberToObject(response, "chip_revision", chip_info.revision);
    cJSON_AddNumberToObject(response, "cores", chip_info.cores);
    
    char *response_str = cJSON_PrintUnformatted(response);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response_str, strlen(response_str));
    
    free(response_str);
    cJSON_Delete(response);
    
    return ESP_OK;
}