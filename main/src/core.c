#include "core.h"
#include "platform.h"
#include "module.h"
#include "config.h"
#include "logger.h"
#include "esp_system.h"
#include <string.h>

core_t g_core;

void core_init(void) {
    g_core.state = SYSTEM_STATE_BOOT;
    g_core.uptime_ms = 0;
    g_core.module_count = 0;
    snprintf(g_core.version, sizeof(g_core.version),
             "%d.%d.%d", CORE_VERSION_MAJOR, CORE_VERSION_MINOR, CORE_VERSION_PATCH);

    LOG_INFO("CORE", "=== MOY CONTROLLER v%s ===", g_core.version);

    platform_info_t* p = platform_detect();
    LOG_INFO("CORE", "Platform: %s", platform_get_name(p->chip));
    LOG_INFO("CORE", "CPU: %d MHz, Free RAM: %d KB",
             p->cpu_freq_mhz, p->free_ram_kb);

    config_load();
    module_register_builtins();
    module_init_all();

    g_core.state = SYSTEM_STATE_RUNNING;
    LOG_INFO("CORE", "System ready! Heap: %d KB", esp_get_free_heap_size() / 1024);
}

void core_update(void) {
    g_core.uptime_ms += 10;
    g_core.heap_free = esp_get_free_heap_size() / 1024;
    module_update_all();
}

void core_reboot(void) {
    LOG_WARN("CORE", "Rebooting system...");
    g_core.state = SYSTEM_STATE_REBOOT;
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_restart();
}

system_state_t core_get_state(void) {
    return g_core.state;
}

const char* core_get_version(void) {
    return g_core.version;
}