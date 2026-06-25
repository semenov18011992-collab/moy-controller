#include "module.h"
#include "logger.h"
#include <string.h>
#include <stdlib.h>

#define MAX_MODULES 32
static module_t* g_modules[MAX_MODULES];
static int g_module_count = 0;

// Внешние объявления модулей
extern module_t module_gpio;
extern module_t module_web;
extern module_t module_test;

bool module_register(module_t* mod) {
    if (g_module_count >= MAX_MODULES) {
        LOG_ERROR("MODULE", "Too many modules! Max: %d", MAX_MODULES);
        return false;
    }
    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i]->name, mod->name) == 0) {
            LOG_WARN("MODULE", "Module '%s' already registered", mod->name);
            return false;
        }
    }
    g_modules[g_module_count++] = mod;
    mod->state = MODULE_STATE_LOADED;
    LOG_INFO("MODULE", "Registered: %s v%s", mod->name, mod->version);
    return true;
}

bool module_init_all(void) {
    LOG_INFO("MODULE", "Initializing all modules...");
    for (int i = 0; i < g_module_count; i++) {
        module_t* mod = g_modules[i];
        if (mod->init) {
            if (!mod->init()) {
                LOG_ERROR("MODULE", "Failed to init: %s", mod->name);
                mod->state = MODULE_STATE_ERROR;
                return false;
            }
            mod->state = MODULE_STATE_INITIALIZED;
            LOG_INFO("MODULE", "Init OK: %s", mod->name);
        }
    }
    return true;
}

bool module_start_all(void) {
    LOG_INFO("MODULE", "Starting all modules...");
    for (int i = 0; i < g_module_count; i++) {
        module_t* mod = g_modules[i];
        if (mod->start) {
            if (!mod->start()) {
                LOG_ERROR("MODULE", "Failed to start: %s", mod->name);
                mod->state = MODULE_STATE_ERROR;
                return false;
            }
            mod->state = MODULE_STATE_RUNNING;
            LOG_INFO("MODULE", "Start OK: %s", mod->name);
        }
    }
    return true;
}

void module_update_all(void) {
    for (int i = 0; i < g_module_count; i++) {
        module_t* mod = g_modules[i];
        if (mod->state == MODULE_STATE_RUNNING && mod->update) {
            mod->update();
        }
    }
}

void module_stop_all(void) {
    LOG_INFO("MODULE", "Stopping all modules...");
    for (int i = g_module_count - 1; i >= 0; i--) {
        module_t* mod = g_modules[i];
        if (mod->stop) mod->stop();
        mod->state = MODULE_STATE_UNLOADED;
    }
}

module_t* module_get(const char* name) {
    for (int i = 0; i < g_module_count; i++) {
        if (strcmp(g_modules[i]->name, name) == 0)
            return g_modules[i];
    }
    return NULL;
}

bool module_is_enabled(const char* name) {
    module_t* mod = module_get(name);
    return mod && mod->state == MODULE_STATE_RUNNING;
}

void module_register_builtins(void) {
    LOG_INFO("MODULE", "Registering built-in modules...");
    module_register(&module_gpio);
    module_register(&module_web);
    module_register(&module_test);
}