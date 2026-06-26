#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include "module.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern module_t sensor_module;

char* sensor_export_json(void);
char* sensor_export_history_json(uint8_t pin_id, int points);
char* sensor_set_pin(uint8_t pin_id, float value);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MODULE_H