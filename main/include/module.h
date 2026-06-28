#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Структура модуля
typedef struct module_s {
    const char* name;
    esp_err_t (*init)(void);
    esp_err_t (*start)(void);
    void (*update)(void);
    esp_err_t (*stop)(void);
    bool enabled;
    void* user_data;
} module_t;

#define MAX_MODULES 16

// Регистрация модуля
bool module_register(module_t* mod);

// Инициализация всех модулей
esp_err_t module_init_all(void);

// Запуск всех модулей
esp_err_t module_start_all(void);

// Обновление всех модулей
void module_update_all(void);

// Остановка всех модулей
esp_err_t module_stop_all(void);

// Регистрация встроенных модулей
void module_register_builtins(void);   // ← ДОБАВЛЕНО

#ifdef __cplusplus
}
#endif