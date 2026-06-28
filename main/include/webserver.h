#pragma once
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Запуск веб-сервера
 * @return true если успешно
 */
bool webserver_start(void);

/**
 * @brief Остановка веб-сервера
 */
void webserver_stop(void);

/**
 * @brief Проверка, запущен ли сервер
 */
bool webserver_is_running(void);

#ifdef __cplusplus
}
#endif