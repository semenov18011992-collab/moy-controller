#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "driver/gpio.h"

#include "platform.h"
#include "core.h"
#include "webserver.h"
#include "config_manager.h"
#include "module.h"
#include "wifi_manager.h"      // ← ЭТОЙ СТРОКИ НЕ ХВАТАЕТ!

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "=== MOY Controller v4.0 ===");
    
    // Включаем синий светодиод
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << 2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_conf);
    gpio_set_level(2, 1);
    ESP_LOGI(TAG, "💡 Blue LED ON (GPIO2)");
    
    // 1. Определение платформы
    platform_info_t* p = platform_detect();
    if (!p) {
        ESP_LOGE(TAG, "Failed to detect platform!");
        return;
    }
    
    ESP_LOGI(TAG, "Running on: %s", p->name);
    ESP_LOGI(TAG, "  CPU: %d MHz", p->cpu_freq_mhz);
    ESP_LOGI(TAG, "  Max pins: %d", p->max_pins);
    ESP_LOGI(TAG, "  Wi-Fi: %s", p->has_wifi ? "✅" : "❌");
    ESP_LOGI(TAG, "  BLE: %s", p->has_ble ? "✅" : "❌");
    
    // 2. Инициализация NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS init error, erasing...");
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // 3. Сетевой стек
    if (p->has_wifi || p->has_ethernet) {
        ESP_LOGI(TAG, "Initializing network stack...");
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        
        if (p->has_wifi) {
            ESP_LOGI(TAG, "Wi-Fi interface created");
            wifi_init_sta();   // ← теперь работает
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 4. Ядро
    core_init();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 5. Конфигурация
    esp_err_t cfg_err = config_manager_init();
    if (cfg_err != ESP_OK) {
        ESP_LOGW(TAG, "Config manager init failed: %s", esp_err_to_name(cfg_err));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 6. Модули
    module_register_builtins();
    module_init_all();
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 7. Веб-сервер
    if (p->has_wifi || p->has_ethernet) {
        bool webserver_ok = webserver_start();
        if (!webserver_ok) {
            ESP_LOGE(TAG, "❌ Failed to start webserver!");
        } else {
            ESP_LOGI(TAG, "✅ Webserver started successfully");
        }
    }
    
    ESP_LOGI(TAG, "✅ System ready on %s!", p->name);
    
    while (1) {
        core_update();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}