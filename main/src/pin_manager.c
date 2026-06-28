#include "pin_manager.h"
#include "esp_log.h"
#include "module_gpio.h"    // Ваш существующий драйвер
#include "pcf8574.h"        // Ваш существующий драйвер
#include <string.h>
#include <stdlib.h>
#include "module.h"

#define TAG "PIN_MGR"

static const char* TAG = "PIN_MGR";
static pin_t pins[MAX_PINS];
static uint16_t pin_count = 0;

// Функция инициализации
static esp_err_t pin_manager_init(void) {
    ESP_LOGI(TAG, "Pin Manager initialized");
    return ESP_OK;
}

// Функция обновления (вызывается в цикле)
static void pin_manager_update(void) {
    // Пока пусто, позже добавим логику
}

bool pin_manager_init(void) {
    ESP_LOGI(TAG, "Pin Manager initialized (overlay mode)");
    // Инициализируем массив
    memset(pins, 0, sizeof(pins));
    pin_count = 0;
    return true;
}

uint16_t pin_add(const pin_t* pin) {
    if (pin_count >= MAX_PINS) {
        ESP_LOGE(TAG, "Max pins reached");
        return 0;
    }
    pins[pin_count] = *pin;
    pins[pin_count].id = pin_count + 1;
    pin_count++;
    ESP_LOGI(TAG, "Pin added: %s (id=%d)", pin->name, pin_count);
    return pin_count;
}

pin_t* pin_get(uint16_t id) {
    if (id == 0 || id > pin_count) return NULL;
    return &pins[id - 1];
}

bool pin_set_value(uint16_t id, float value) {
    pin_t* pin = pin_get(id);
    if (!pin) return false;
    
    // Используем существующие функции драйверов
    switch(pin->type) {
        case PIN_TYPE_GPIO_OUT:
            gpio_set_level(pin->physical_pin, value > 0);
            break;
        case PIN_TYPE_GPIO_IN:
            // Только чтение, не пишем
            break;
        default:
            ESP_LOGW(TAG, "Unsupported pin type for set: %d", pin->type);
            return false;
    }
    pin->value = value;
    pin->last_update = esp_log_timestamp();
    return true;
}

float pin_get_value(uint16_t id) {
    pin_t* pin = pin_get(id);
    if (!pin) return 0.0f;
    
    // Обновляем значение из драйвера
    if (pin->type == PIN_TYPE_GPIO_IN) {
        pin->value = gpio_get_level(pin->physical_pin);
        pin->last_update = esp_log_timestamp();
    }
    return pin->value;
}

char* pin_export_json(void) {

    // Определение модуля
module_t pin_manager_module = {
    .name = "pin_manager",
    .version = "v1.0",
    .init = pin_manager_init,
    .start = NULL,
    .update = pin_manager_update,
};

    // Формируем JSON со всеми пинами
    // ... (реализация по аналогии с sensor_module)
    char* json = malloc(1024);
    sprintf(json, "{\"pins\":[]}");
    return json;
}