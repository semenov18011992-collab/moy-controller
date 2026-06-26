#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include "module.h"

#ifdef __cplusplus
extern "C" {
#endif

extern module_t sensor_module;
char* sensor_export_json(void);

#ifdef __cplusplus
}
#endif

#endif // SENSOR_MODULE_H