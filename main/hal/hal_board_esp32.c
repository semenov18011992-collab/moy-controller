#include "hal_board.h"
#include "esp_log.h"

static void esp32_init_wifi(void) {
    ESP_LOGI("HAL", "ESP32 Wi-Fi init (built-in)");
    // здесь стандартная инициализация Wi-Fi для ESP32
}

static void esp32_init_ethernet(void) {
    ESP_LOGI("HAL", "ESP32 Ethernet init");
    // инициализация Ethernet для ESP32
}

static void esp32_init_gpio(void) {
    // стандартная инициализация GPIO
}

static void esp32_init_i2c(void) {
    // стандартная инициализация I2C
}

board_hal_t hal_board_esp32 = {
    .init_wifi = esp32_init_wifi,
    .init_ethernet = esp32_init_ethernet,
    .init_gpio = esp32_init_gpio,
    .init_i2c = esp32_init_i2c,
    .board_name = "ESP32"
};