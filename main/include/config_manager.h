#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "pin_config.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Инициализация
esp_err_t config_manager_init(void);

// Работа с конфигурацией пина
esp_err_t config_save_pin(const pin_config_t *cfg);
esp_err_t config_load_pin(uint8_t pin, pin_config_t *cfg);
esp_err_t config_delete_pin(uint8_t pin);
esp_err_t config_apply_pin(const pin_config_t *cfg);
esp_err_t config_load_all(pin_config_t *configs, uint8_t *count);

// Получение значения пина
float config_get_pin_value(uint8_t pin, bool is_output);

// Вспомогательные функции уже в pin_config.h

#ifdef __cplusplus
}
#endif

#endif // CONFIG_MANAGER_H