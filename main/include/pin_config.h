#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PIN_NAME_LEN    32
#define MAX_PINS            40
#define CONFIG_NAMESPACE    "moy_cfg"

typedef enum {
    PIN_TYPE_SENSOR = 0,
    PIN_TYPE_ACTUATOR = 1,
    PIN_TYPE_INPUT = 2,
    PIN_TYPE_DISABLED = 3
} pin_type_t;

typedef enum {
    PIN_MODE_DIGITAL = 0,
    PIN_MODE_ANALOG = 1,
    PIN_MODE_ONEWIRE = 2
} pin_mode_t;

typedef struct {
    uint8_t pin;                    // Номер GPIO
    char name[MAX_PIN_NAME_LEN];    // Имя пина
    uint8_t type;                   // pin_type_t
    uint8_t mode;                   // pin_mode_t
    float min_val;                  // Минимальное значение
    float max_val;                  // Максимальное значение
    uint8_t pullup;                 // 0=disabled, 1=enabled
    uint8_t inverted;               // 0=normal, 1=inverted
    uint8_t enabled;                // 0=disabled, 1=enabled
    uint32_t crc;                   // Контрольная сумма для проверки
} pin_config_t;

// Вспомогательные функции
const char* pin_type_to_string(pin_type_t type);
pin_type_t pin_string_to_type(const char* str);
const char* pin_mode_to_string(pin_mode_t mode);
pin_mode_t pin_string_to_mode(const char* str);

#ifdef __cplusplus
}
#endif

#endif // PIN_CONFIG_H