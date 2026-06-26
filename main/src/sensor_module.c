#include "module.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "SENSOR"
#define TRACKED_PINS 4
#define HISTORY_SIZE 100

typedef struct {
    uint32_t timestamp;
    float value;
} history_point_t;

typedef struct {
    uint8_t pin_id;
    char name[64];
    float current_value;
    history_point_t history[HISTORY_SIZE];
    int history_count;
} sensor_pin_data_t;

static sensor_pin_data_t tracked_pins[TRACKED_PINS] = {
    {.pin_id = 34, .name = "Temp Kotla", .current_value = 45.0, .history_count = 0},
    {.pin_id = 35, .name = "Temp Ulicy", .current_value = -5.0, .history_count = 0},
    {.pin_id = 26, .name = "Rele Kotla", .current_value = 1.0, .history_count = 0},
    {.pin_id = 27, .name = "Nasos", .current_value = 0.0, .history_count = 0}
};

static bool initialized = false;

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
    static char json_buf[2048];
    char* ptr = json_buf;
    int remaining = sizeof(json_buf);
    int written;

    written = snprintf(ptr, remaining, "{\"pins\":[");
    if (written < 0) return "{\"error\":\"buffer overflow\"}";
    ptr += written;
    remaining -= written;

    for (int i = 0; i < TRACKED_PINS; i++) {
        written = snprintf(ptr, remaining,
            "{\"id\":%d,\"name\":\"%s\",\"value\":%.1f,\"online\":true}",
            tracked_pins[i].pin_id, tracked_pins[i].name, tracked_pins[i].current_value);
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
            tracked_pins[i].current_value = value;
            snprintf(response, sizeof(response), "{\"success\":true,\"pin\":%d,\"value\":%.1f}", pin_id, value);
            return response;
        }
    }
    snprintf(response, sizeof(response), "{\"error\":\"pin %d not found\"}", pin_id);
    return response;
}

static bool sensor_init(void) {
    if (initialized) return true;
    
    srand((unsigned)time(NULL));
    for (int i = 0; i < TRACKED_PINS; i++) {
        tracked_pins[i].current_value = 20.0 + (float)(rand() % 100) / 5.0;
        tracked_pins[i].history_count = 0;
        for (int j = 0; j < 20; j++) {
            add_history_point(&tracked_pins[i], 
                20.0 + (float)(rand() % 100) / 5.0);
        }
    }
    initialized = true;
    ESP_LOGI(TAG, "Sensor module initialized (SCADA mode)");
    return true;
}

static bool sensor_start(void) {
    ESP_LOGI(TAG, "Sensor module started (SCADA mode)");
    return true;
}

static void sensor_update(void) {
    if (!initialized) return;
    
    static uint32_t last_update = 0;
    uint32_t now = xTaskGetTickCount();
    
    if (now - last_update < pdMS_TO_TICKS(2000)) return;
    last_update = now;
    
    for (int i = 0; i < TRACKED_PINS; i++) {
        float delta = ((float)(rand() % 20) - 10.0) / 10.0;
        float new_value = tracked_pins[i].current_value + delta;
        
        if (i < 2) {
            if (new_value < -20) new_value = -20;
            if (new_value > 60) new_value = 60;
        } else {
            new_value = new_value > 0.5 ? 1.0 : 0.0;
        }
        add_history_point(&tracked_pins[i], new_value);
    }
}

module_t sensor_module = {
    .name = "sensor_min",
    .version = "v1.0.0",
    .init = sensor_init,
    .start = sensor_start,
    .update = sensor_update
};