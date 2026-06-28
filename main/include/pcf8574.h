#ifndef PCF8574_H
#define PCF8574_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Сканирование PCF8574 на шине I2C
bool pcf8574_probe(uint8_t addr);

// Чтение данных с PCF8574
uint8_t pcf8574_read(uint8_t addr);

// Запись данных в PCF8574 (возвращает ESP_OK или ошибку)
esp_err_t pcf8574_write(uint8_t addr, uint8_t data);  // <-- исправлено на esp_err_t

#ifdef __cplusplus
}
#endif

#endif // PCF8574_H