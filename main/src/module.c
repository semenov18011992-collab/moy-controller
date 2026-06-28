#include "module.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "MODULE";

static module_t* modules[MAX_MODULES];
static int module_count = 0;

bool module_register(module_t* mod) {
    if (!mod || module_count >= MAX_MODULES) {
        return false;
    }
    
    // Проверяем, нет ли уже такого модуля
    for (int i = 0; i < module_count; i++) {
        if (modules[i] == mod) {
            ESP_LOGW(TAG, "Module %s already registered", mod->name);
            return false;
        }
    }
    
    modules[module_count++] = mod;
    ESP_LOGI(TAG, "Registered module: %s", mod->name);
    return true;
}

esp_err_t module_init_all(void) {
    ESP_LOGI(TAG, "Initializing all modules...");
    
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (!mod) continue;
        
        if (mod->init) {
            esp_err_t err = mod->init();
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Init OK: %s", mod->name);
            } else {
                ESP_LOGE(TAG, "Init FAILED: %s (error: %s)", 
                         mod->name, esp_err_to_name(err));
            }
        }
    }
    
    return ESP_OK;
}

esp_err_t module_start_all(void) {
    ESP_LOGI(TAG, "Starting all modules...");
    
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (!mod) continue;
        
        if (mod->start) {
            esp_err_t err = mod->start();
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Start OK: %s", mod->name);
            } else {
                ESP_LOGE(TAG, "Start FAILED: %s (error: %s)", 
                         mod->name, esp_err_to_name(err));
            }
        }
    }
    
    return ESP_OK;
}

void module_update_all(void) {
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (!mod) continue;
        
        if (mod->update) {
            mod->update();
        }
    }
}

esp_err_t module_stop_all(void) {
    ESP_LOGI(TAG, "Stopping all modules...");
    
    for (int i = 0; i < module_count; i++) {
        module_t* mod = modules[i];
        if (!mod) continue;
        
        if (mod->stop) {
            esp_err_t err = mod->stop();
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Stop OK: %s", mod->name);
            } else {
                ESP_LOGE(TAG, "Stop FAILED: %s (error: %s)", 
                         mod->name, esp_err_to_name(err));
            }
        }
    }
    
    return ESP_OK;
}

// ============================================
// РЕГИСТРАЦИЯ ВСТРОЕННЫХ МОДУЛЕЙ
// ============================================
void module_register_builtins(void) {
    extern module_t gpio_module;
    extern module_t web_module;
    extern module_t test_module;
    extern module_t sensor_module;
    extern module_t pin_manager_module;
    
    module_register(&gpio_module);
    module_register(&web_module);
    module_register(&test_module);
    module_register(&sensor_module);
    module_register(&pin_manager_module);
    
    ESP_LOGI(TAG, "All built-in modules registered");
}