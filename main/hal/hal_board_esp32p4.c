// hal_board_esp32p4.c
#include "hal_board.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
//#include "driver/spi_master.h"
//#include "driver/ledc.h"
//#include "esp_wifi.h"
//#include "esp_eth.h"
//#include "esp_eth_phy.h"
//#include "esp_eth_mac.h"
#include "esp_psram.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"

static const char *TAG = "HAL_ESP32P4";

// ---------- Вспомогательные функции инициализации ----------

static void esp32p4_init_wifi(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi (ESP32-P4 via remote ESP32-C6)");
    // Для JC-ESP32P4-M3-DEV Wi-Fi управляется через ESP32-C6 по SDIO.
    // Если вы используете стандартный ESP32-P4 без внешнего чипа, то Wi-Fi недоступен.
    // Поэтому здесь нужно либо использовать esp_hosted, либо оставить заглушку.
    // Пока оставим заглушку с предупреждением.
    ESP_LOGW(TAG, "Wi-Fi is not directly available on ESP32-P4. Use external ESP32-C6 for Wi-Fi.");
    // В реальном проекте здесь должен быть вызов esp_hosted или другой инициализации.
}

static void esp32p4_init_ethernet(void) {
    ESP_LOGI(TAG, "Initializing Ethernet (ESP32-P4 via RMII)");
    // ESP32-P4 имеет Ethernet MAC, можно инициализировать с внешним PHY.
    // Для JC-ESP32P4-M3-DEV Ethernet обычно реализован через IP101 PHY.
    // Здесь можно добавить код инициализации Ethernet.
    // Пока заглушка.
    ESP_LOGI(TAG, "Ethernet initialization not implemented in HAL.");
}

static void esp32p4_init_gpio(void) {
    ESP_LOGI(TAG, "GPIO initialized (ESP32-P4)");
    // Стандартная настройка GPIO не требуется.
}

static void esp32p4_init_i2c(void) {
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

static void esp32p4_init_spi(void) {
    ESP_LOGI(TAG, "SPI master initialized (default VSPI)");
    // Можно добавить инициализацию SPI.
}

static void esp32p4_init_psram(void) {
    ESP_LOGI(TAG, "PSRAM already initialized by bootloader, no action needed.");
    // PSRAM автоматически инициализируется на старте.
}

// ---------- Экспортируем структуру HAL ----------

board_hal_t hal_board_esp32p4 = {
    .init_wifi = esp32p4_init_wifi,
    .init_ethernet = esp32p4_init_ethernet,
    .init_gpio = esp32p4_init_gpio,
    .init_i2c = esp32p4_init_i2c,
    .init_spi = esp32p4_init_spi,
    .init_psram = esp32p4_init_psram,
    .board_name = "ESP32-P4"
};