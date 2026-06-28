#include "module.h"
#include "esp_log.h"

static const char *TAG = "TEST_MODULE";

static esp_err_t test_init(void) {
    ESP_LOGI(TAG, "Test module init");
    return ESP_OK;
}

static esp_err_t test_start(void) {
    ESP_LOGI(TAG, "Test module start");
    return ESP_OK;
}

static void test_update(void) {
    // Обновление тестового модуля
}

static esp_err_t test_stop(void) {
    ESP_LOGI(TAG, "Test module stop");
    return ESP_OK;
}

// Регистрация модуля
module_t test_module = {
    .name = "test",
    .init = test_init,
    .start = test_start,
    .update = test_update,
    .stop = test_stop,
    .enabled = true
};