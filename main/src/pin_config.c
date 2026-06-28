#include "pin_config.h"
#include <string.h>

pin_type_t pin_string_to_type(const char* str) {
    if (strcmp(str, "actuator") == 0) return PIN_TYPE_ACTUATOR;
    if (strcmp(str, "input") == 0) return PIN_TYPE_INPUT;
    if (strcmp(str, "disabled") == 0) return PIN_TYPE_DISABLED;
    return PIN_TYPE_SENSOR;
}

pin_mode_t pin_string_to_mode(const char* str) {
    if (strcmp(str, "analog") == 0) return PIN_MODE_ANALOG;
    if (strcmp(str, "onewire") == 0) return PIN_MODE_ONEWIRE;
    return PIN_MODE_DIGITAL;
}

const char* pin_type_to_string(pin_type_t type) {
    switch (type) {
        case PIN_TYPE_SENSOR:   return "sensor";
        case PIN_TYPE_ACTUATOR: return "actuator";
        case PIN_TYPE_INPUT:    return "input";
        case PIN_TYPE_DISABLED: return "disabled";
        default:                return "sensor";
    }
}

const char* pin_mode_to_string(pin_mode_t mode) {
    switch (mode) {
        case PIN_MODE_DIGITAL: return "digital";
        case PIN_MODE_ANALOG:  return "analog";
        case PIN_MODE_ONEWIRE: return "onewire";
        default:               return "digital";
    }
}