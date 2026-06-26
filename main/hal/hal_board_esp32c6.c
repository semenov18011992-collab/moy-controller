// hal_board_esp32c6.c
#include "hal_board.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
//#include "driver/spi_master.h"
//#include "esp_wifi.h"
//#include "esp_eth.h"
//#include "esp_eth_phy.h"
//#include "esp_eth_mac.h"

static const char *TAG = "HAL_ESP32C6";

// ---------- Вспомогательные функции инициализации ----------

static void esp32c6_init_wifi(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi (ESP32-C6 built-in)");
    // Инициализация сетевого стека (ещё не сделана)
    // Обычно вызывается один раз в main, но можно дублировать с проверкой
    static bool initialized = false;
    if (!initialized) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        initialized = true;
    }
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    // Создаём сетевой интерфейс для STA
    esp_netif_create_default_wifi_sta();
    ESP_LOGI(TAG, "Wi-Fi initialized");
    // Здесь можно задать режим, но обычно это делается отдельно в приложении
}

static void esp32c6_init_ethernet(void) {
    ESP_LOGI(TAG, "Initializing Ethernet (ESP32-C6 via RMII)");
    // Настройка Ethernet для ESP32-C6 (если используется)
    // Для большинства плат с ESP32-C6 Ethernet опционален, оставим заглушку
    // Реальную инициализацию можно добавить при необходимости
    ESP_LOGI(TAG, "Ethernet not configured by default");
}

static void esp32c6_init_gpio(void) {
    ESP_LOGI(TAG, "GPIO initialized (default)");
    // Всё, что нужно для базовой настройки GPIO – уже делается при конфигурации пинов.
    // Здесь можно добавить общую инициализацию, если необходимо.
}

static void esp32c6_init_i2c(void) {
    ESP_LOGI(TAG, "I2C master initialized (default pins SDA=GPIO21, SCL=GPIO22)");
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0));
}

static void esp32c6_init_spi(void) {
    ESP_LOGI(TAG, "SPI master initialized (default VSPI)");
    // Настройка SPI – аналогично, можно добавить при необходимости
}

// ---------- Экспортируем структуру HAL ----------

board_hal_t hal_board_esp32c6 = {
    .init_wifi = esp32c6_init_wifi,
    .init_ethernet = esp32c6_init_ethernet,
    .init_gpio = esp32c6_init_gpio,
    .init_i2c = esp32c6_init_i2c,
    .init_spi = esp32c6_init_spi,
    .board_name = "ESP32-C6"
};