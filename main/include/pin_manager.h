#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "module.h"

#define MAX_PINS 256

typedef enum {
    PIN_TYPE_UNUSED = 0,
    PIN_TYPE_GPIO_IN,
    PIN_TYPE_GPIO_OUT,
    PIN_TYPE_ANALOG_IN,
    PIN_TYPE_ANALOG_OUT,
    PIN_TYPE_PWM,
    PIN_TYPE_1WIRE,
    PIN_TYPE_VIRTUAL
} pin_type_t;

typedef struct {
    uint16_t id;
    char name[32];
    char description[64];
    pin_type_t type;
    uint8_t physical_pin;    // 0-39 или 255
    uint8_t pcf8574_addr;    // 0x20-0x27 или 0
    uint8_t pcf8574_pin;     // 0-7 или 255
    bool inverted;
    float value;
    float min_value;
    float max_value;
    float calibration;
    uint32_t last_update;
    bool enabled;
    bool online;
} pin_t;

bool pin_manager_init(void);
uint16_t pin_add(const pin_t* pin);
pin_t* pin_get(uint16_t id);
pin_t* pin_get_by_name(const char* name);
bool pin_set_value(uint16_t id, float value);
float pin_get_value(uint16_t id);
char* pin_export_json(void);

extern module_t pin_manager_module;