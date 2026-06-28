#include "core.h"
#include "platform.h"
#include "config.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "CORE";

void core_init(void) {
    ESP_LOGI(TAG, "Core initialization started");
    
    // Определяем платформу
    platform_info_t* p = platform_detect();
    if (p == NULL) {
        ESP_LOGE(TAG, "Failed to detect platform");
        return;
    }
    
    // Выводим информацию о платформе
    ESP_LOGI(TAG, "Platform: %s", p->name);
    ESP_LOGI(TAG, "CPU: %d MHz", p->cpu_freq_mhz);
    ESP_LOGI(TAG, "Max pins: %d", p->max_pins);
    
    // Получаем свободную RAM
    size_t free_ram = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Free RAM: %zu KB", free_ram / 1024);
    
    // Загружаем конфигурацию
    if (!config_load()) {
        ESP_LOGW(TAG, "Failed to load config, using defaults");
    }
    
    ESP_LOGI(TAG, "Core initialized successfully");
}

void core_update(void) {
    // Обновление состояния ядра
    // Здесь можно добавить периодические задачи
    static uint32_t counter = 0;
    counter++;
    if (counter % 1000 == 0) {
        ESP_LOGD(TAG, "Core update: %lu", counter);
    }
}

void core_loop(void) {
    // Основной цикл ядра (если нужен)
}