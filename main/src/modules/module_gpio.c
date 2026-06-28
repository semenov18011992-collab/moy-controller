#include "module.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "pcf8574.h"
#include "platform.h"
#include "esp_err.h"

static const char *TAG = "GPIO_MODULE";

// Простая структура для управления пинами
typedef struct {
    uint8_t pin;
    bool is_output;
    bool initialized;
    uint8_t value; 
} gpio_handle_t;

static gpio_handle_t gpio_pins[64];
static int gpio_count = 0;

// ============================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================
esp_err_t gpio_module_init(void) {
    ESP_LOGI(TAG, "Initializing GPIO module");
    
    platform_info_t* p = platform_detect();
    if (!p) {
        ESP_LOGE(TAG, "Failed to detect platform");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Platform: %s, Max pins: %d", p->name, p->max_pins);
    
    // Инициализация GPIO
    for (int i = 0; i < p->max_pins && i < 64; i++) {
        gpio_pins[i].pin = i;
        gpio_pins[i].is_output = false;
        gpio_pins[i].initialized = false;
    }
    gpio_count = p->max_pins;
    
    return ESP_OK;
}

// ============================================
// НАСТРОЙКА ПИНА
// ============================================
esp_err_t gpio_pin_set_mode(uint8_t pin, bool output) {
    if (pin >= gpio_count) {
        ESP_LOGE(TAG, "Pin %d out of range (max %d)", pin, gpio_count);
        return ESP_ERR_INVALID_ARG;
    }
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        gpio_pins[pin].is_output = output;
        gpio_pins[pin].initialized = true;
        ESP_LOGD(TAG, "Pin %d configured as %s", pin, output ? "OUTPUT" : "INPUT");
    }
    
    return err;
}

// ============================================
//  УСТАНОВКА ЗНАЧЕНИЯ ПИНА
// ============================================
esp_err_t gpio_pin_set_value(uint8_t pin, bool value) {
    if (pin >= gpio_count || !gpio_pins[pin].initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!gpio_pins[pin].is_output) {
        ESP_LOGW(TAG, "Pin %d is not configured as output", pin);
        return ESP_ERR_INVALID_STATE;
    }
    
    gpio_set_level(pin, value ? 1 : 0);
    gpio_pins[pin].value = value ? 255 : 0;
    return ESP_OK;
}

// ============================================
// ЧТЕНИЕ ЗНАЧЕНИЯ ПИНА
// ============================================
int gpio_pin_get_value(uint8_t pin) {
    if (pin >= gpio_count || !gpio_pins[pin].initialized) {
        return -1;
    }
    
    return gpio_get_level(pin);
}
// ============================================
// РЕГИСТРАЦИЯ МОДУЛЯ
// ============================================
module_t gpio_module = {
    .name = "gpio",
    .init = gpio_module_init,
    .start = NULL,
    .update = NULL,
    .stop = NULL,
    .enabled = true
};