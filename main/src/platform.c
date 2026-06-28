#include "platform.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "PLATFORM";

// Статическая структура с информацией о платформе
static platform_info_t platform_info = {
    .chip = PLATFORM_UNKNOWN,
    .name = "Unknown",
    .cpu_freq_mhz = 0,
    .max_pins = 0,
    .has_wifi = false,
    .has_ble = false,
    .has_ethernet = false,
    .has_ppa = false,
    .has_ai_accel = false,
    .has_psram = false,
    .has_usb_host = false,
    .has_i2s = false,
    .has_ledc = false,
    .has_mipi_dsi = false,
    .support_modbus = false,
    .support_mqtt = false,
    .support_audio = false,
    .support_video = false
};

platform_info_t* platform_detect(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // Определяем тип чипа и его возможности
    switch (chip_info.model) {
        case CHIP_ESP32:
            platform_info.chip = PLATFORM_ESP32;
            platform_info.name = "ESP32";
            platform_info.cpu_freq_mhz = 240;
            platform_info.max_pins = 40;
            platform_info.has_wifi = true;
            platform_info.has_ble = true;
            platform_info.has_ethernet = false;
            platform_info.has_ppa = false;
            platform_info.has_ai_accel = false;
            platform_info.has_psram = false;
            platform_info.has_usb_host = false;
            platform_info.has_i2s = true;
            platform_info.has_ledc = true;
            platform_info.has_mipi_dsi = false;
            platform_info.support_modbus = true;
            platform_info.support_mqtt = true;
            platform_info.support_audio = true;
            platform_info.support_video = false;
            break;
            
        case CHIP_ESP32S3:
            platform_info.chip = PLATFORM_ESP32_S3;
            platform_info.name = "ESP32-S3";
            platform_info.cpu_freq_mhz = 240;
            platform_info.max_pins = 45;
            platform_info.has_wifi = true;
            platform_info.has_ble = true;
            platform_info.has_ethernet = false;
            platform_info.has_ppa = false;
            platform_info.has_ai_accel = false;
            platform_info.has_psram = true;
            platform_info.has_usb_host = true;
            platform_info.has_i2s = true;
            platform_info.has_ledc = true;
            platform_info.has_mipi_dsi = false;
            platform_info.support_modbus = true;
            platform_info.support_mqtt = true;
            platform_info.support_audio = true;
            platform_info.support_video = false;
            break;
            
        case CHIP_ESP32C3:
            platform_info.chip = PLATFORM_ESP32_C3;
            platform_info.name = "ESP32-C3";
            platform_info.cpu_freq_mhz = 160;
            platform_info.max_pins = 22;
            platform_info.has_wifi = true;
            platform_info.has_ble = true;
            platform_info.has_ethernet = false;
            platform_info.has_ppa = false;
            platform_info.has_ai_accel = false;
            platform_info.has_psram = false;
            platform_info.has_usb_host = false;
            platform_info.has_i2s = false;
            platform_info.has_ledc = true;
            platform_info.has_mipi_dsi = false;
            platform_info.support_modbus = false;
            platform_info.support_mqtt = true;
            platform_info.support_audio = false;
            platform_info.support_video = false;
            break;
            
        case CHIP_ESP32C6:
            platform_info.chip = PLATFORM_ESP32_C6;
            platform_info.name = "ESP32-C6";
            platform_info.cpu_freq_mhz = 160;
            platform_info.max_pins = 28;
            platform_info.has_wifi = true;
            platform_info.has_ble = true;
            platform_info.has_ethernet = false;
            platform_info.has_ppa = false;
            platform_info.has_ai_accel = false;
            platform_info.has_psram = false;
            platform_info.has_usb_host = false;
            platform_info.has_i2s = false;
            platform_info.has_ledc = true;
            platform_info.has_mipi_dsi = false;
            platform_info.support_modbus = false;
            platform_info.support_mqtt = true;
            platform_info.support_audio = false;
            platform_info.support_video = false;
            break;
            
        case CHIP_ESP32P4:
            platform_info.chip = PLATFORM_ESP32_P4;
            platform_info.name = "ESP32-P4";
            platform_info.cpu_freq_mhz = 400;
            platform_info.max_pins = 50;
            platform_info.has_wifi = false;      // НЕТ встроенного Wi-Fi
            platform_info.has_ble = false;       // НЕТ встроенного BLE
            platform_info.has_ethernet = true;
            platform_info.has_ppa = true;        // ЕСТЬ PPA
            platform_info.has_ai_accel = true;   // ЕСТЬ AI ускоритель
            platform_info.has_psram = true;
            platform_info.has_usb_host = true;
            platform_info.has_i2s = true;
            platform_info.has_ledc = true;
            platform_info.has_mipi_dsi = true;   // ЕСТЬ MIPI DSI
            platform_info.support_modbus = true;
            platform_info.support_mqtt = true;
            platform_info.support_audio = true;
            platform_info.support_video = true;  // Поддерживает видео
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown platform: %d", chip_info.model);
            platform_info.chip = PLATFORM_UNKNOWN;
            platform_info.name = "Unknown";
            platform_info.cpu_freq_mhz = 0;
            platform_info.max_pins = 40;
            platform_info.has_wifi = false;
            platform_info.has_ble = false;
            platform_info.has_ethernet = false;
            platform_info.has_ppa = false;
            platform_info.has_ai_accel = false;
            platform_info.has_psram = false;
            platform_info.has_usb_host = false;
            platform_info.has_i2s = false;
            platform_info.has_ledc = false;
            platform_info.has_mipi_dsi = false;
            platform_info.support_modbus = false;
            platform_info.support_mqtt = false;
            platform_info.support_audio = false;
            platform_info.support_video = false;
            break;
    }
    
    ESP_LOGI(TAG, "Platform: %s, CPU: %d MHz, Pins: %d", 
             platform_info.name, 
             platform_info.cpu_freq_mhz, 
             platform_info.max_pins);
    ESP_LOGI(TAG, "  Wi-Fi: %s, BLE: %s, PPA: %s, AI: %s",
             platform_info.has_wifi ? "✅" : "❌",
             platform_info.has_ble ? "✅" : "❌",
             platform_info.has_ppa ? "✅" : "❌",
             platform_info.has_ai_accel ? "✅" : "❌");
    
    return &platform_info;
}

platform_type_t platform_get_type(void) {
    return platform_info.chip;
}

bool platform_has_feature(const char* feature) {
    if (strcmp(feature, "wifi") == 0) return platform_info.has_wifi;
    if (strcmp(feature, "ble") == 0) return platform_info.has_ble;
    if (strcmp(feature, "ethernet") == 0) return platform_info.has_ethernet;
    if (strcmp(feature, "ppa") == 0) return platform_info.has_ppa;
    if (strcmp(feature, "ai_accel") == 0) return platform_info.has_ai_accel;
    if (strcmp(feature, "psram") == 0) return platform_info.has_psram;
    if (strcmp(feature, "usb_host") == 0) return platform_info.has_usb_host;
    if (strcmp(feature, "i2s") == 0) return platform_info.has_i2s;
    if (strcmp(feature, "ledc") == 0) return platform_info.has_ledc;
    if (strcmp(feature, "mipi_dsi") == 0) return platform_info.has_mipi_dsi;
    if (strcmp(feature, "modbus") == 0) return platform_info.support_modbus;
    if (strcmp(feature, "mqtt") == 0) return platform_info.support_mqtt;
    if (strcmp(feature, "audio") == 0) return platform_info.support_audio;
    if (strcmp(feature, "video") == 0) return platform_info.support_video;
    return false;
}