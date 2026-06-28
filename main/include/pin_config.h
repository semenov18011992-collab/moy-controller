#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PINS 64
#define MAX_PIN_NAME_LEN 32

typedef enum {
    PIN_TYPE_SENSOR = 0,
    PIN_TYPE_ACTUATOR,
    PIN_TYPE_INPUT,
    PIN_TYPE_DISABLED
} pin_type_t;

typedef enum {
    PIN_MODE_DIGITAL = 0,
    PIN_MODE_ANALOG,
    PIN_MODE_ONEWIRE
} pin_mode_t;

typedef struct {
    uint8_t pin;
    char name[MAX_PIN_NAME_LEN];
    pin_type_t type;
    pin_mode_t mode;
    uint8_t enabled:1;
    uint8_t pullup:1;
    uint8_t inverted:1;
    float min_val;
    float max_val;
    uint16_t crc;
} pin_config_t;

// Конвертация типов (НЕ static!)
pin_type_t pin_string_to_type(const char* str);
pin_mode_t pin_string_to_mode(const char* str);
const char* pin_type_to_string(pin_type_t type);
const char* pin_mode_to_string(pin_mode_t mode);

#ifdef __cplusplus
}
#endif