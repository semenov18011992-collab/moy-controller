#pragma once
#include "esp_err.h"
#include "pin_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================
// ОПРЕДЕЛЕНИЯ
// ============================================
#define CONFIG_NAMESPACE "pin_config"
#define MAX_PINS 64

// ============================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================
/**
 * @brief Инициализация менеджера конфигурации
 * @return ESP_OK при успехе
 */
esp_err_t config_manager_init(void);

// ============================================
// ОСНОВНЫЕ ФУНКЦИИ
// ============================================
/**
 * @brief Сохранение конфигурации пина
 */
esp_err_t config_save_pin(const pin_config_t *cfg);

/**
 * @brief Загрузка конфигурации пина
 */
esp_err_t config_load_pin(uint8_t pin, pin_config_t *cfg);

/**
 * @brief Загрузка всех конфигураций
 */
esp_err_t config_load_all(pin_config_t *configs, uint8_t *count);

/**
 * @brief Удаление конфигурации пина
 */
esp_err_t config_delete_pin(uint8_t pin);

/**
 * @brief Применение конфигурации пина
 */
esp_err_t config_apply_pin(const pin_config_t *cfg);

/**
 * @brief Загрузка конфигурации с проверкой CRC
 */
esp_err_t config_load_pin_verified(uint8_t pin, pin_config_t *cfg);

#ifdef __cplusplus
}
#endif