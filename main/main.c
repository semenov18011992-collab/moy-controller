#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "core.h"
#include "module.h"
#include "logger.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "sensor_module.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "pin_config.h"

// Объявляем TAG для логирования
static const char *TAG = "MAIN";

void app_main(void) {
    // Инициализация NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // ===== ИНИЦИАЛИЗАЦИЯ CONFIG_MANAGER (ДО core_init) =====
    ESP_LOGI(TAG, "Initializing config manager...");
    esp_err_t cfg_err = config_manager_init();
    if (cfg_err != ESP_OK) {
        ESP_LOGE(TAG, "Config manager init failed: %s", esp_err_to_name(cfg_err));
    }

    // Инициализация ядра
    core_init();

    // Инициализация сетевого стека (для HTTP-сервера)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // === Wi-Fi ===
    wifi_manager_init();

    // === РЕГИСТРАЦИЯ МОДУЛЯ sensor_module ===
    // Если sensor_module регистрируется через module_register_builtins() в core.c,
    // то здесь регистрировать НЕ НУЖНО, чтобы избежать двойной регистрации.
    // Если не регистрируется в core.c — раскомментируйте:
    // module_register(&sensor_module);

    // === ЗАПУСК МОДУЛЕЙ ===
    module_start_all();

    // ===== ЗАГРУЗКА И ПРИМЕНЕНИЕ СОХРАНЁННЫХ КОНФИГУРАЦИЙ (ПОСЛЕ ЗАПУСКА МОДУЛЕЙ) =====
    ESP_LOGI(TAG, "Applying saved pin configurations...");
    pin_config_t configs[MAX_PINS];
    uint8_t count = 0;
    if (config_load_all(configs, &count) == ESP_OK) {
        for (int i = 0; i < count; i++) {
            if (configs[i].enabled) {
                config_apply_pin(&configs[i]);
                ESP_LOGI(TAG, "Applied config for pin %d: %s", configs[i].pin, configs[i].name);
            }
        }
        ESP_LOGI(TAG, "Applied %d pin configurations", count);
    } else {
        ESP_LOGI(TAG, "No saved pin configurations found, using defaults");
    }

    LOG_INFO("MAIN", "System ready!");
    ESP_LOGI(TAG, "✅ System ready! Heap: %d KB", esp_get_free_heap_size() / 1024);

    // Основной цикл
    while (1) {
        core_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}