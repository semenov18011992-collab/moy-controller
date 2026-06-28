#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Структура конфигурации
typedef struct {
    bool enable_test_module;
    bool enable_gpio_module;
    bool enable_web_module;
    int test_value;
    // Добавьте другие поля по мере необходимости
} config_t;

/**
 * @brief Получение указателя на конфигурацию
 */
config_t* config_get(void);

/**
 * @brief Получение значения конфигурации по ключу
 */
int config_get_int(const char *key, int default_value);

/**
 * @brief Загрузка конфигурации
 */
bool config_load(void);

/**
 * @brief Сохранение конфигурации
 */
bool config_save(void);

#ifdef __cplusplus
}
#endif