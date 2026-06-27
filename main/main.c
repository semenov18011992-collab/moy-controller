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

void app_main(void) {
    // Инициализация NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

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

    LOG_INFO("MAIN", "System ready!");

    // Основной цикл
    while (1) {
        core_update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}