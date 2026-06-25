#include "config.h"
#include "logger.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

static config_t g_config = {
    .wifi_ssid = "MyController",
    .wifi_password = "12345678",
    .device_name = "MOY-CONTROLLER",
    .web_port = 80,
    .enable_test_module = true
};

void config_load(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("config", NVS_READONLY, &handle);
    if (err == ESP_OK) {
        size_t len;
        nvs_get_str(handle, "wifi_ssid", g_config.wifi_ssid, &len);
        nvs_get_str(handle, "wifi_pass", g_config.wifi_password, &len);
        nvs_get_str(handle, "dev_name", g_config.device_name, &len);
        uint16_t port;
        if (nvs_get_u16(handle, "web_port", &port) == ESP_OK)
            g_config.web_port = port;
        uint8_t test_en;
        if (nvs_get_u8(handle, "test_en", &test_en) == ESP_OK)
            g_config.enable_test_module = (test_en != 0);
        nvs_close(handle);
        LOG_INFO("CONFIG", "Configuration loaded");
    } else {
        LOG_WARN("CONFIG", "No config found, using defaults");
        config_save();
    }
}

void config_save(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("config", NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_str(handle, "wifi_ssid", g_config.wifi_ssid);
        nvs_set_str(handle, "wifi_pass", g_config.wifi_password);
        nvs_set_str(handle, "dev_name", g_config.device_name);
        nvs_set_u16(handle, "web_port", g_config.web_port);
        nvs_set_u8(handle, "test_en", g_config.enable_test_module ? 1 : 0);
        nvs_commit(handle);
        nvs_close(handle);
        LOG_INFO("CONFIG", "Configuration saved");
    }
}

config_t* config_get(void) {
    return &g_config;
}