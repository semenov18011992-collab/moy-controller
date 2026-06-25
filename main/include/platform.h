#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PLATFORM_UNKNOWN = 0,
    PLATFORM_ESP32,
    PLATFORM_ESP32_S2,
    PLATFORM_ESP32_S3,
    PLATFORM_ESP32_C3,
    PLATFORM_ESP32_C6,
    PLATFORM_ESP32_P4,
    PLATFORM_ESP32_H2
} platform_chip_t;

typedef enum {
    PERF_LOW = 0,
    PERF_MEDIUM,
    PERF_HIGH,
    PERF_ULTRA
} performance_profile_t;

typedef struct {
    platform_chip_t chip;
    performance_profile_t profile;
    uint32_t cpu_freq_mhz;
    uint32_t flash_size_mb;
    uint32_t psram_size_mb;
    uint32_t free_ram_kb;
    bool has_ethernet;
    bool has_ble;
    bool has_wifi_6;
    bool has_usb_otg;
    bool has_ai_accelerator;
    bool has_i2s;
    bool has_mipi_dsi;
} platform_info_t;

platform_info_t* platform_detect(void);
const char* platform_get_name(platform_chip_t chip);
const char* platform_get_profile_name(performance_profile_t profile);
bool platform_has_feature(const char* feature);

#endif