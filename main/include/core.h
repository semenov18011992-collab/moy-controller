#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Инициализация ядра системы
 */
void core_init(void);

/**
 * @brief Обновление состояния ядра (вызывается в главном цикле)
 */
void core_update(void);

/**
 * @brief Основной цикл ядра
 */
void core_loop(void);

#ifdef __cplusplus
}
#endif