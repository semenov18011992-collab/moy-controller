#include "platform.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>

static platform_info_t g_platform;

platform_info_t* platform_detect(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    memset(&g_platform, 0, sizeof(g_platform));

    // Определение чипа
    switch (chip_info.model) {
        case CHIP_ESP32:
            g_platform.chip = PLATFORM_ESP32;
            g_platform.cpu_freq_mhz = 160;
            g_platform.has_ethernet = true;
            g_platform.has_ble = true;
            g_platform.has_i2s = true;
            g_platform.has_usb_otg = false;
            g_platform.has_wifi_6 = false;
            g_platform.has_ai_accelerator = false;
            g_platform.has_mipi_dsi = false;
            break;
        case CHIP_ESP32C6:
            g_platform.chip = PLATFORM_ESP32_C6;
            g_platform.cpu_freq_mhz = 160;
            g_platform.has_ethernet = true;
            g_platform.has_ble = true;
            g_platform.has_wifi_6 = true;
            g_platform.has_usb_otg = true;
            g_platform.has_i2s = true;
            g_platform.has_ai_accelerator = false;
            g_platform.has_mipi_dsi = false;
            break;
        case CHIP_ESP32P4:
            g_platform.chip = PLATFORM_ESP32_P4;
            g_platform.cpu_freq_mhz = 400;
            g_platform.has_ethernet = true;
            g_platform.has_ble = true;
            g_platform.has_wifi_6 = true;
            g_platform.has_usb_otg = true;
            g_platform.has_ai_accelerator = true;
            g_platform.has_mipi_dsi = true;
            g_platform.has_i2s = true;
            break;
        default:
            g_platform.chip = PLATFORM_UNKNOWN;
            g_platform.cpu_freq_mhz = 80;
            break;
    }

    // Профиль производительности
    if (g_platform.chip == PLATFORM_ESP32_P4)
        g_platform.profile = PERF_ULTRA;
    else if (g_platform.cpu_freq_mhz >= 240)
        g_platform.profile = PERF_HIGH;
    else if (g_platform.cpu_freq_mhz >= 160)
        g_platform.profile = PERF_MEDIUM;
    else
        g_platform.profile = PERF_LOW;

    // Размер флеша (упрощённо, для совместимости)
    g_platform.flash_size_mb = 4;   // для ESP32-C6 обычно 4 МБ
    g_platform.psram_size_mb = 0;   // можно определить через psram
    g_platform.free_ram_kb = esp_get_free_heap_size() / 1024;

    ESP_LOGI("PLATFORM", "=== ОПРЕДЕЛЕНА ПЛАТФОРМА ===");
    ESP_LOGI("PLATFORM", "Chip: %s", platform_get_name(g_platform.chip));
    ESP_LOGI("PLATFORM", "Profile: %s", platform_get_profile_name(g_platform.profile));
    ESP_LOGI("PLATFORM", "CPU: %d MHz", g_platform.cpu_freq_mhz);
    ESP_LOGI("PLATFORM", "Free RAM: %d KB", g_platform.free_ram_kb);
    ESP_LOGI("PLATFORM", "WiFi 6: %s", g_platform.has_wifi_6 ? "YES" : "NO");
    ESP_LOGI("PLATFORM", "BLE: %s", g_platform.has_ble ? "YES" : "NO");
    ESP_LOGI("PLATFORM", "USB OTG: %s", g_platform.has_usb_otg ? "YES" : "NO");

    return &g_platform;
}

const char* platform_get_name(platform_chip_t chip) {
    switch (chip) {
        case PLATFORM_ESP32: return "ESP32";
        case PLATFORM_ESP32_S2: return "ESP32-S2";
        case PLATFORM_ESP32_S3: return "ESP32-S3";
        case PLATFORM_ESP32_C3: return "ESP32-C3";
        case PLATFORM_ESP32_C6: return "ESP32-C6";
        case PLATFORM_ESP32_P4: return "ESP32-P4";
        case PLATFORM_ESP32_H2: return "ESP32-H2";
        default: return "UNKNOWN";
    }
}

const char* platform_get_profile_name(performance_profile_t profile) {
    switch (profile) {
        case PERF_LOW: return "LOW";
        case PERF_MEDIUM: return "MEDIUM";
        case PERF_HIGH: return "HIGH";
        case PERF_ULTRA: return "ULTRA";
        default: return "UNKNOWN";
    }
}

bool platform_has_feature(const char* feature) {
    platform_info_t* p = &g_platform;
    if (strcmp(feature, "ethernet") == 0) return p->has_ethernet;
    if (strcmp(feature, "ble") == 0) return p->has_ble;
    if (strcmp(feature, "wifi6") == 0) return p->has_wifi_6;
    if (strcmp(feature, "usb_otg") == 0) return p->has_usb_otg;
    if (strcmp(feature, "ai") == 0) return p->has_ai_accelerator;
    if (strcmp(feature, "i2s") == 0) return p->has_i2s;
    if (strcmp(feature, "mipi") == 0) return p->has_mipi_dsi;
    return false;
}