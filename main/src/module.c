#include "module.h"
#include "esp_log.h"
#include <string.h>

#define TAG "MODULE"
#define MAX_MODULES 16

static module_t* modules[MAX_MODULES];
static int module_count = 0;

bool module_register(module_t* mod) {
    if (module_count >= MAX_MODULES) {
        ESP_LOGE(TAG, "Module registry full!");
        return false;
    }
    if (!mod || !mod->name) {
        ESP_LOGE(TAG, "Invalid module!");
        return false;
    }
    for (int i = 0; i < module_count; i++) {
        if (strcmp(modules[i]->name, mod->name) == 0) {
            ESP_LOGW(TAG, "Module '%s' already registered!", mod->name);
            return false;
        }
    }
    modules[module_count++] = mod;
    ESP_LOGI(TAG, "Registered: %s %s", mod->name, mod->version ? mod->version : "");
    return true;
}

bool module_init_all(void) {
    ESP_LOGI(TAG, "Initializing all modules...");
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (mod->init) {
            if (mod->init()) {
                ESP_LOGI(TAG, "Init OK: %s", mod->name);
            } else {
                ESP_LOGE(TAG, "Init FAILED: %s", mod->name);
                return false;
            }
        }
    }
    return true;
}

bool module_start_all(void) {
    ESP_LOGI(TAG, "Starting all modules...");
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (mod->start) {
            if (mod->start()) {
                ESP_LOGI(TAG, "Start OK: %s", mod->name);
            } else {
                ESP_LOGE(TAG, "Start FAILED: %s", mod->name);
                return false;
            }
        }
    }
    return true;
}

void module_update_all(void) {
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (mod->update) {
            mod->update();
        }
    }
}

void module_register_builtins(void) {
    // Внешние объявления модулей
    extern module_t gpio_module;
    extern module_t web_module;
    extern module_t test_module;
    extern module_t sensor_module;
    extern module_t pin_manager_module;   // ← НОВАЯ СТРОКА
    
    module_register(&gpio_module);
    module_register(&web_module);
    module_register(&test_module);
    module_register(&sensor_module);
    module_register(&pin_manager_module); // ← НОВАЯ СТРОКА
}