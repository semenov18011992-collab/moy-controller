#include "module.h"
#include "esp_log.h"

static const char *TAG = "WEB_MODULE";

static esp_err_t web_init(void) {
    ESP_LOGI(TAG, "Web module init");
    return ESP_OK;
}

static esp_err_t web_start(void) {
    ESP_LOGI(TAG, "Web module start");
    return ESP_OK;
}

static void web_update(void) {
    // Обновление веб-модуля
}

static esp_err_t web_stop(void) {
    ESP_LOGI(TAG, "Web module stop");
    return ESP_OK;
}

// Регистрация модуля
module_t web_module = {
    .name = "web",
    .init = web_init,
    .start = web_start,
    .update = web_update,
    .stop = web_stop,
    .enabled = true
};