#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Типы платформ
typedef enum {
    PLATFORM_ESP32 = 0,
    PLATFORM_ESP32_S3,
    PLATFORM_ESP32_C3,
    PLATFORM_ESP32_C6,
    PLATFORM_ESP32_P4,
    PLATFORM_UNKNOWN
} platform_type_t;

// Возможности платформы
typedef struct {
    platform_type_t chip;
    const char* name;
    int cpu_freq_mhz;
    int max_pins;
    
    // Аппаратные возможности (флаги)
    bool has_wifi;
    bool has_ble;
    bool has_ethernet;
    bool has_ppa;           // Programmable Peripheral Accelerator (ESP32-P4)
    bool has_ai_accel;      // AI ускоритель (ESP32-P4)
    bool has_psram;
    bool has_usb_host;
    bool has_i2s;
    bool has_ledc;
    bool has_mipi_dsi;      // ESP32-P4
    
    // Поддерживаемые модули
    bool support_modbus;
    bool support_mqtt;
    bool support_audio;
    bool support_video;
} platform_info_t;

/**
 * @brief Определение платформы
 * @return Указатель на структуру с информацией о платформе
 */
platform_info_t* platform_detect(void);

/**
 * @brief Получение типа платформы
 */
platform_type_t platform_get_type(void);

/**
 * @brief Проверка наличия возможности
 */
bool platform_has_feature(const char* feature);

#ifdef __cplusplus
}
#endif