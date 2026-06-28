#include "config.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

static const char *TAG = "CONFIG";

// Глобальная конфигурация
static config_t g_config = {
    .enable_test_module = true,
    .enable_gpio_module = true,
    .enable_web_module = true,
    .test_value = 0
};

config_t* config_get(void) {
    return &g_config;
}

int config_get_int(const char *key, int default_value) {
    if (strcmp(key, "test_value") == 0) {
        return g_config.test_value;
    }
    if (strcmp(key, "enable_test_module") == 0) {
        return g_config.enable_test_module ? 1 : 0;
    }
    if (strcmp(key, "enable_gpio_module") == 0) {
        return g_config.enable_gpio_module ? 1 : 0;
    }
    if (strcmp(key, "enable_web_module") == 0) {
        return g_config.enable_web_module ? 1 : 0;
    }
    return default_value;
}

bool config_load(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("config", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No config in NVS, using defaults");
        return true;
    }
    
    // Загружаем значения (используем int32_t для совместимости)
    int32_t val32;
    
    err = nvs_get_i32(handle, "test_value", &val32);
    if (err == ESP_OK) {
        g_config.test_value = (int)val32;
    }
    
    uint8_t val8;
    err = nvs_get_u8(handle, "test_module", &val8);
    if (err == ESP_OK) {
        g_config.enable_test_module = (val8 != 0);
    }
    
    err = nvs_get_u8(handle, "gpio_module", &val8);
    if (err == ESP_OK) {
        g_config.enable_gpio_module = (val8 != 0);
    }
    
    err = nvs_get_u8(handle, "web_module", &val8);
    if (err == ESP_OK) {
        g_config.enable_web_module = (val8 != 0);
    }
    
    nvs_close(handle);
    return true;
}

bool config_save(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("config", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS");
        return false;
    }
    
    // Сохраняем значения
    nvs_set_i32(handle, "test_value", (int32_t)g_config.test_value);
    nvs_set_u8(handle, "test_module", g_config.enable_test_module ? 1 : 0);
    nvs_set_u8(handle, "gpio_module", g_config.enable_gpio_module ? 1 : 0);
    nvs_set_u8(handle, "web_module", g_config.enable_web_module ? 1 : 0);
    
    nvs_commit(handle);
    nvs_close(handle);
    
    return true;
}