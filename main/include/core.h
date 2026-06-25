#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CORE_VERSION_MAJOR 1
#define CORE_VERSION_MINOR 0
#define CORE_VERSION_PATCH 0

typedef enum {
    SYSTEM_STATE_BOOT = 0,
    SYSTEM_STATE_INIT,
    SYSTEM_STATE_RUNNING,
    SYSTEM_STATE_ERROR,
    SYSTEM_STATE_REBOOT
} system_state_t;

typedef struct {
    system_state_t state;
    uint32_t uptime_ms;
    uint32_t heap_free;
    char version[16];
    void* modules[32];
    int module_count;
} core_t;

extern core_t g_core;

void core_init(void);
void core_update(void);
void core_reboot(void);
system_state_t core_get_state(void);
const char* core_get_version(void);

#endif