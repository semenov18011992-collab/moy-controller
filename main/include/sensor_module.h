#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Экспорт всех пинов в JSON
 */
char* sensor_export_json(void);

/**
 * @brief Экспорт истории пина в JSON
 */
char* sensor_export_history_json(uint8_t pin, int points);

/**
 * @brief Установка значения пина
 */
char* sensor_set_pin(uint8_t pin, float value);

/**
 * @brief Получение значения пина
 */
float sensor_get_pin_value(uint8_t pin);

#ifdef __cplusplus
}
#endif