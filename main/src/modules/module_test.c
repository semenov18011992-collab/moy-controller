#include "module.h"
#include "logger.h"
#include "config.h"
#include "driver/gpio.h"

#define TEST_LED_PIN GPIO_NUM_2

static bool test_init(void) {
    config_t* cfg = config_get();
    if (!cfg->enable_test_module) {
        LOG_INFO("TEST", "Test module disabled in config");
        return true;
    }
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << TEST_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    LOG_INFO("TEST", "Test module init");
    return true;
}

static bool test_start(void) { 
    LOG_INFO("TEST", "Started"); 
    return true; 
}

static void test_update(void) {
    config_t* cfg = config_get();
    if (!cfg->enable_test_module) return;
    static bool state = false;
    state = !state;
    gpio_set_level(TEST_LED_PIN, state ? 1 : 0);
}

static void test_stop(void) {
    gpio_set_level(TEST_LED_PIN, 0);
    LOG_INFO("TEST", "Stopped");
}

// В конце файла — исправленная структура
module_t test_module = {
    .name = "test",
    .version = "v1.0.0",
    .init = test_init,
    .start = test_start,
    .update = test_update,
    .stop = test_stop,
 
};