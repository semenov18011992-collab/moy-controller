#include "module.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"      // <-- добавить для GPIO_NUM_4
#include "ds18b20.h"

#define TAG "SENSOR"
#define TRACKED_PINS 16
#define HISTORY_SIZE 100
#define DS18B20_GPIO GPIO_NUM_4

typedef struct {
    uint32_t timestamp;
    float value;
} history_point_t;

typedef struct {
    uint8_t pin_id;
    char name[64];
    char type[32];
    float current_value;
    history_point_t history[HISTORY_SIZE];
    int history_count;
    bool is_output;
} sensor_pin_data_t;

static sensor_pin_data_t tracked_pins[TRACKED_PINS] = {
    {.pin_id = 34, .name = "Температура котла", .type = "sensor", .is_output = false, .current_value = 45.0, .history_count = 0},
    {.pin_id = 35, .name = "Температура улицы", .type = "sensor", .is_output = false, .current_value = -5.0, .history_count = 0},
    {.pin_id = 36, .name = "Температура ГВС", .type = "sensor", .is_output = false, .current_value = 55.0, .history_count = 0},
    {.pin_id = 37, .name = "Давление", .type = "sensor", .is_output = false, .current_value = 2.5, .history_count = 0},
    {.pin_id = 38, .name = "Расход", .type = "sensor", .is_output = false, .current_value = 1.2, .history_count = 0},
    {.pin_id = 39, .name = "Влажность", .type = "sensor", .is_output = false, .current_value = 45.0, .history_count = 0},
    {.pin_id = 26, .name = "Реле котла", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 27, .name = "Насос", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 28, .name = "Клапан ГВС", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 29, .name = "Вентилятор", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 30, .name = "Насос 2", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 31, .name = "Подогрев", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 32, .name = "Сирена", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 33, .name = "Свет", .type = "actuator", .is_output = true, .current_value = 0.0, .history_count = 0},
    {.pin_id = 25, .name = "Датчик протечки", .type = "input", .is_output = false, .current_value = 0.0, .history_count = 0},
    {.pin_id = 24, .name = "Датчик дыма", .type = "input", .is_output = false, .current_value = 0.0, .history_count = 0},
};

static bool initialized = false;
static bool use_real_sensors = false;

static void add_history_point(sensor_pin_data_t* data, float value) {
    if (data->history_count < HISTORY_SIZE) {
        data->history[data->history_count].timestamp = (uint32_t)time(NULL);
        data->history[data->history_count].value = value;
        data->history_count++;
    } else {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            data->history[i] = data->history[i + 1];
        }
        data->history[HISTORY_SIZE - 1].timestamp = (uint32_t)time(NULL);
        data->history[HISTORY_SIZE - 1].value = value;
    }
    data->current_value = value;
}

char* sensor_export_json(void) {
    static char json_buf[4096];
    char* ptr = json_buf;
    int remaining = sizeof(json_buf);
    int written;

    written = snprintf(ptr, remaining, "{\"pins\":[");
    if (written < 0) return "{\"error\":\"buffer overflow\"}";
    ptr += written;
    remaining -= written;

    for (int i = 0; i < TRACKED_PINS; i++) {
        written = snprintf(ptr, remaining,
            "{\"id\":%d,\"name\":\"%s\",\"value\":%.1f,\"online\":true,\"type\":\"%s\",\"is_output\":%s}",
            tracked_pins[i].pin_id, tracked_pins[i].name, tracked_pins[i].current_value,
            tracked_pins[i].type, tracked_pins[i].is_output ? "true" : "false");
        if (written < 0) return "{\"error\":\"buffer overflow\"}";
        ptr += written;
        remaining -= written;
        if (i < TRACKED_PINS - 1) {
            written = snprintf(ptr, remaining, ",");
            if (written < 0) return "{\"error\":\"buffer overflow\"}";
            ptr += written;
            remaining -= written;
        }
    }

    snprintf(ptr, remaining, "]}");
    return json_buf;
}

char* sensor_export_history_json(uint8_t pin_id, int points) {
    static char json_buf[4096];
    char* ptr = json_buf;
    int remaining = sizeof(json_buf);
    int written;

    sensor_pin_data_t* data = NULL;
    for (int i = 0; i < TRACKED_PINS; i++) {
        if (tracked_pins[i].pin_id == pin_id) {
            data = &tracked_pins[i];
            break;
        }
    }
    if (!data) return "{\"error\":\"pin not found\"}";

    written = snprintf(ptr, remaining, "[");
    if (written < 0) return "{\"error\":\"buffer overflow\"}";
    ptr += written;
    remaining -= written;

    int start = data->history_count > points ? data->history_count - points : 0;
    for (int i = start; i < data->history_count; i++) {
        written = snprintf(ptr, remaining,
            "{\"t\":%lu,\"v\":%.1f}",
            (unsigned long)data->history[i].timestamp, data->history[i].value);
        if (written < 0) return "{\"error\":\"buffer overflow\"}";
        ptr += written;
        remaining -= written;
        if (i < data->history_count - 1) {
            written = snprintf(ptr, remaining, ",");
            if (written < 0) return "{\"error\":\"buffer overflow\"}";
            ptr += written;
            remaining -= written;
        }
    }

    snprintf(ptr, remaining, "]");
    return json_buf;
}

char* sensor_set_pin(uint8_t pin_id, float value) {
    static char response[64];
    for (int i = 0; i < TRACKED_PINS; i++) {
        if (tracked_pins[i].pin_id == pin_id) {
            if (tracked_pins[i].is_output) {
                tracked_pins[i].current_value = value;
                snprintf(response, sizeof(response), "{\"success\":true,\"pin\":%d,\"value\":%.1f}", pin_id, value);
                return response;
            } else {
                snprintf(response, sizeof(response), "{\"error\":\"pin %d is not output\"}", pin_id);
                return response;
            }
        }
    }
    snprintf(response, sizeof(response), "{\"error\":\"pin %d not found\"}", pin_id);
    return response;
}

static bool sensor_init(void) {
    if (initialized) return true;
    
    int sensor_count = ds18b20_init(DS18B20_GPIO);
    
    if (sensor_count > 0) {
        use_real_sensors = true;
        int max_pins = (sensor_count < TRACKED_PINS) ? sensor_count : TRACKED_PINS;
        for (int i = 0; i < max_pins; i++) {
            float temp = ds18b20_read_temp_by_index(i);
            if (temp > -100 && temp < 125) {
                tracked_pins[i].current_value = temp;
            }
        }
        ESP_LOGI(TAG, "Found %d DS18B20 sensors", sensor_count);
    }
    
    srand((unsigned)time(NULL));
    for (int i = 0; i < TRACKED_PINS; i++) {
        if (tracked_pins[i].is_output) {
            tracked_pins[i].current_value = 0.0;
        } else if (strcmp(tracked_pins[i].type, "input") == 0) {
            tracked_pins[i].current_value = 0.0;
        } else {
            tracked_pins[i].current_value = 20.0 + (float)(rand() % 100) / 5.0;
        }
        tracked_pins[i].history_count = 0;
        for (int j = 0; j < 20; j++) {
            add_history_point(&tracked_pins[i], tracked_pins[i].current_value);
        }
    }
    
    initialized = true;
    ESP_LOGI(TAG, "Sensor module initialized (%d pins, %s)", TRACKED_PINS, use_real_sensors ? "DS18B20" : "emulation");
    return true;
}

static bool sensor_start(void) {
    ESP_LOGI(TAG, "Sensor module started");
    return true;
}

static void sensor_update(void) {
    if (!initialized) return;
    
    static uint32_t last_update = 0;
    uint32_t now = xTaskGetTickCount();
    
    if (now - last_update < pdMS_TO_TICKS(2000)) return;
    last_update = now;
    
    if (!use_real_sensors) {
        for (int i = 0; i < TRACKED_PINS; i++) {
            if (tracked_pins[i].is_output) continue;
            if (strcmp(tracked_pins[i].type, "input") == 0) {
                tracked_pins[i].current_value = (float)(rand() % 2);
                add_history_point(&tracked_pins[i], tracked_pins[i].current_value);
                continue;
            }
            float delta = ((float)(rand() % 20) - 10.0) / 10.0;
            float new_value = tracked_pins[i].current_value + delta;
            if (new_value < -20) new_value = -20;
            if (new_value > 60) new_value = 60;
            add_history_point(&tracked_pins[i], new_value);
        }
    }
}

module_t sensor_module = {
    .name = "sensor_min",
    .version = "v1.0.0",
    .init = sensor_init,
    .start = sensor_start,
    .update = sensor_update
};   // <-- точка с запятой обязательна