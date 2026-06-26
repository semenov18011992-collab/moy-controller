#include "module.h"
#include "logger.h"
#include "platform.h"
#include "pcf8574.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

typedef enum {
    PIN_SOURCE_INTERNAL = 0,
    PIN_SOURCE_PCF8574
} pin_source_t;

typedef enum {
    PIN_MODE_DISABLED = 0,
    PIN_MODE_INPUT,
    PIN_MODE_OUTPUT,
    PIN_MODE_ANALOG,
    PIN_MODE_PWM,
    PIN_MODE_1WIRE
} pin_mode_t;

typedef struct {
    uint8_t id;
    char name[32];
    pin_source_t source;
    uint8_t pin_number;
    uint8_t pcf8574_address;
    pin_mode_t mode;
    float value;
    float min_value;
    float max_value;
    bool inverted;
    bool enabled;
    bool is_online;
    uint32_t last_update;
} gpio_pin_t;

#define MAX_PINS 256
static gpio_pin_t* g_pins[MAX_PINS];
static int g_pin_count = 0;

static const uint8_t PCF8574_ADDRESSES[] = {0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27};
#define PCF8574_MAX 8
static struct { uint8_t address; bool present; uint8_t output_state; } g_pcf_devices[PCF8574_MAX];
static int g_pcf_count = 0;

static void detect_pcf8574_devices(void) {
    LOG_INFO("GPIO", "Scanning for PCF8574...");
    for (int i = 0; i < PCF8574_MAX; i++) {
        uint8_t addr = PCF8574_ADDRESSES[i];
        if (pcf8574_probe(addr)) {
            g_pcf_devices[g_pcf_count].address = addr;
            g_pcf_devices[g_pcf_count].present = true;
            g_pcf_devices[g_pcf_count].output_state = 0x00;
            g_pcf_count++;
            LOG_INFO("GPIO", "  ✅ Found PCF8574 at 0x%02X", addr);
            for (int pin = 0; pin < 8; pin++) {
                gpio_pin_t* new_pin = calloc(1, sizeof(gpio_pin_t));
                new_pin->id = g_pin_count + 1;
                new_pin->source = PIN_SOURCE_PCF8574;
                new_pin->pcf8574_address = addr;
                new_pin->pin_number = pin;
                new_pin->mode = PIN_MODE_INPUT;
                new_pin->enabled = true;
                new_pin->is_online = true;
                snprintf(new_pin->name, sizeof(new_pin->name), "PCF_%02X_PIN%d", addr, pin);
                g_pins[g_pin_count++] = new_pin;
            }
        } else {
            LOG_DEBUG("GPIO", "  ❌ No device at 0x%02X", addr);
        }
    }
    LOG_INFO("GPIO", "Found %d PCF8574 expanders", g_pcf_count);
}

static void init_internal_pins(void) {
    platform_info_t* p = platform_detect();
    int max_pins = 40;
    if (p->chip == PLATFORM_ESP32_C6) max_pins = 28;
    else if (p->chip == PLATFORM_ESP32_P4) max_pins = 50;
    LOG_INFO("GPIO", "Initializing %d internal pins", max_pins);
    for (int pin = 0; pin < max_pins; pin++) {
        gpio_pin_t* new_pin = calloc(1, sizeof(gpio_pin_t));
        new_pin->id = g_pin_count + 1;
        new_pin->source = PIN_SOURCE_INTERNAL;
        new_pin->pin_number = pin;
        new_pin->mode = PIN_MODE_INPUT;
        new_pin->enabled = true;
        new_pin->is_online = true;
        snprintf(new_pin->name, sizeof(new_pin->name), "GPIO_%d", pin);
        g_pins[g_pin_count++] = new_pin;
    }
}

bool gpio_pin_set_mode(uint8_t pin_id, pin_mode_t mode) {
    if (pin_id >= g_pin_count) return false;
    gpio_pin_t* pin = g_pins[pin_id];
    pin->mode = mode;
    if (pin->source == PIN_SOURCE_INTERNAL) {
        gpio_config_t io_conf = { .pin_bit_mask = (1ULL << pin->pin_number) };
        switch (mode) {
            case PIN_MODE_INPUT:
            case PIN_MODE_1WIRE:
                io_conf.mode = GPIO_MODE_INPUT;
                io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
                break;
            case PIN_MODE_OUTPUT:
                io_conf.mode = GPIO_MODE_OUTPUT;
                break;
            case PIN_MODE_PWM:
                io_conf.mode = GPIO_MODE_OUTPUT;
                break;
            case PIN_MODE_ANALOG:
                io_conf.mode = GPIO_MODE_INPUT;
                break;
            default:
                io_conf.mode = GPIO_MODE_INPUT;
                break;
        }
        io_conf.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io_conf);
        if (mode == PIN_MODE_PWM) {
            ledc_timer_config_t timer = { .speed_mode = LEDC_LOW_SPEED_MODE,
                                          .timer_num = LEDC_TIMER_0,
                                          .duty_resolution = 10,
                                          .freq_hz = 1000,
                                          .clk_cfg = LEDC_AUTO_CLK };
            ledc_timer_config(&timer);
            ledc_channel_config_t chan = { .speed_mode = LEDC_LOW_SPEED_MODE,
                                           .channel = LEDC_CHANNEL_0,
                                           .timer_sel = LEDC_TIMER_0,
                                           .intr_type = LEDC_INTR_DISABLE,
                                           .gpio_num = pin->pin_number,
                                           .duty = 0 };
            ledc_channel_config(&chan);
        }
    }
    return true;
}

bool gpio_pin_set_value(uint8_t pin_id, float value) {
    if (pin_id >= g_pin_count) return false;
    gpio_pin_t* pin = g_pins[pin_id];
    if (!pin->enabled) return false;
    pin->value = value;
    if (pin->inverted) value = 1.0f - value;
    if (pin->source == PIN_SOURCE_INTERNAL) {
        if (pin->mode == PIN_MODE_OUTPUT)
            gpio_set_level(pin->pin_number, value > 0.5f ? 1 : 0);
        else if (pin->mode == PIN_MODE_PWM) {
            int duty = (int)(value / 100.0f * 1023);
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        }
    } else if (pin->source == PIN_SOURCE_PCF8574) {
        int idx = -1;
        for (int i = 0; i < g_pcf_count; i++) {
            if (g_pcf_devices[i].address == pin->pcf8574_address) { idx = i; break; }
        }
        if (idx >= 0) {
            if (value > 0.5f)
                g_pcf_devices[idx].output_state |= (1 << pin->pin_number);
            else
                g_pcf_devices[idx].output_state &= ~(1 << pin->pin_number);
            pcf8574_write(pin->pcf8574_address, g_pcf_devices[idx].output_state);
        }
    }
    return true;
}

float gpio_pin_get_value(uint8_t pin_id) {
    if (pin_id >= g_pin_count) return 0.0f;
    gpio_pin_t* pin = g_pins[pin_id];
    if (!pin->enabled) return 0.0f;
    if (pin->source == PIN_SOURCE_INTERNAL) {
        if (pin->mode == PIN_MODE_INPUT || pin->mode == PIN_MODE_1WIRE)
            pin->value = gpio_get_level(pin->pin_number) ? 1.0f : 0.0f;
        else if (pin->mode == PIN_MODE_ANALOG)
            pin->value = 25.0f; // упрощённо
    } else if (pin->source == PIN_SOURCE_PCF8574) {
        uint8_t state = pcf8574_read(pin->pcf8574_address);
        pin->value = (state & (1 << pin->pin_number)) ? 1.0f : 0.0f;
    }
    if (pin->inverted) pin->value = 1.0f - pin->value;
    pin->last_update = xTaskGetTickCount();
    return pin->value;
}

gpio_pin_t* gpio_pin_get(uint8_t pin_id) {
    if (pin_id >= g_pin_count) return NULL;
    return g_pins[pin_id];
}
int gpio_pin_get_count(void) { return g_pin_count; }

char* gpio_export_json(void) {
    static char buffer[4096];
    buffer[0] = 0;
    strcat(buffer, "{\"pins\":[");
    for (int i = 0; i < g_pin_count; i++) {
        gpio_pin_t* pin = g_pins[i];
        char pin_json[256];
        snprintf(pin_json, sizeof(pin_json),
                "{\"id\":%d,\"name\":\"%s\",\"source\":%d,\"pin\":%d,\"mode\":%d,\"value\":%.2f}",
                pin->id, pin->name, pin->source, pin->pin_number, pin->mode, pin->value);
        strcat(buffer, pin_json);
        if (i < g_pin_count - 1) strcat(buffer, ",");
    }
    strcat(buffer, "]}");
    return buffer;
}

static bool gpio_init(void) {
    LOG_INFO("GPIO", "Init GPIO module (v6.0)");
    memset(g_pins, 0, sizeof(g_pins));
    g_pin_count = 0;
    g_pcf_count = 0;
    init_internal_pins();
    detect_pcf8574_devices();
    LOG_INFO("GPIO", "Total pins: %d", g_pin_count);
    return true;
}

static bool gpio_start(void) { LOG_INFO("GPIO", "Started"); return true; }
static void gpio_update(void) {
    for (int i = 0; i < g_pin_count; i++) {
        gpio_pin_t* pin = g_pins[i];
        if (pin->enabled && pin->mode == PIN_MODE_INPUT)
            gpio_pin_get_value(i);
    }
}
static void gpio_stop(void) { LOG_INFO("GPIO", "Stopped"); }

module_t gpio_module = {
    .name = "gpio",
    .version = "v2.0.0",
    .init = gpio_init,
    .start = gpio_start,
    .update = gpio_update
};