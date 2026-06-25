// hal_board_esp32c6_remote.c
#include "hal_board_esp32c6_remote.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_hosted.h"      // основная библиотека ESP-Hosted
#include "esp_hosted_sdio.h" // для SDIO-транспорта

static const char *TAG = "HAL_REMOTE_WIFI";

// Функция инициализации Wi-Fi через удалённый ESP32-C6
void wifi_remote_init(void) {
    ESP_LOGI(TAG, "Initializing Wi-Fi via ESP-Hosted (SDIO)");

    // 1. Инициализация сетевого стека (если ещё не сделана)
    static bool net_init = false;
    if (!net_init) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        net_init = true;
    }

    // 2. Создание сетевого интерфейса (STA или AP)
    esp_netif_create_default_wifi_sta(); // или esp_netif_create_default_wifi_ap()

    // 3. Инициализация ESP-Hosted
    // Настройка SDIO: пины CLK=GPIO19, CMD=GPIO18, D0=GPIO14, D1=GPIO15, D2=GPIO16, D3=GPIO17
    esp_hosted_sdio_config_t sdio_cfg = ESP_HOSTED_SDIO_DEFAULT_CONFIG();
    sdio_cfg.clk_pin = GPIO_NUM_19;
    sdio_cfg.cmd_pin = GPIO_NUM_18;
    sdio_cfg.d0_pin = GPIO_NUM_14;
    sdio_cfg.d1_pin = GPIO_NUM_15;
    sdio_cfg.d2_pin = GPIO_NUM_16;
    sdio_cfg.d3_pin = GPIO_NUM_17;
    // Дополнительные настройки (например, частота)
    sdio_cfg.freq_khz = 20000;  // 20 MHz

    esp_hosted_sdio_config_t *hosted_cfg = &sdio_cfg;
    ESP_ERROR_CHECK(esp_hosted_init(hosted_cfg));

    // 4. Инициализация Wi-Fi (на удалённой стороне)
    // ESP-Hosted автоматически запускает Wi-Fi на ESP32-C6,
    // но мы можем задать параметры через API.
    // Например, установить режим STA и подключиться к сети.
    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi remote initialization complete");
}

// Опционально: функция для отключения
void wifi_remote_deinit(void) {
    esp_hosted_deinit();
    ESP_LOGI(TAG, "Wi-Fi remote deinitialized");
}