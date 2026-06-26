#include "module.h"
#include "esp_log.h"

#define TAG "SENSOR"

static bool sensor_init(void) {
    ESP_LOGI(TAG, "Sensor module init (minimal)");
    return true;
}

static bool sensor_start(void) {
    ESP_LOGI(TAG, "Sensor module started");
    return true;
}

static void sensor_update(void) {
    // Пока ничего не делаем
}

char* sensor_export_json(void) {
    return "[]";
}

module_t sensor_module = {
    .name = "sensor_min",
    .version = "v1.0.0",
    .init = sensor_init,
    .start = sensor_start,
    .update = sensor_update
};