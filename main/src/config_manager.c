#include "config_manager.h"
#include "pin_config.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_rom_crc.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "CONFIG_MGR";

// Таблица соответствий для преобразования типов
static const struct {
    pin_type_t type;
    const char *str;
} type_map[] = {
    {PIN_TYPE_SENSOR, "sensor"},
    {PIN_TYPE_ACTUATOR, "actuator"},
    {PIN_TYPE_INPUT, "input"},
    {PIN_TYPE_DISABLED, "disabled"}
};

static const struct {
    pin_mode_t mode;
    const char *str;
} mode_map[] = {
    {PIN_MODE_DIGITAL, "digital"},
    {PIN_MODE_ANALOG, "analog"},
    {PIN_MODE_ONEWIRE, "onewire"}
};

// Инициализация менеджера конфигурации
esp_err_t config_manager_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS corruption detected, erasing...");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Config manager initialized");
    return ESP_OK;
}

// Сохранение конфигурации пина
esp_err_t config_save_pin(const pin_config_t *cfg) {
    if (!cfg || cfg->pin >= MAX_PINS) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }
    
    char key[16];
    snprintf(key, sizeof(key), "pin_%d", cfg->pin);
    
    // Вычисляем CRC для проверки целостности
    pin_config_t cfg_copy = *cfg;
    cfg_copy.crc = 0;
    cfg_copy.crc = esp_rom_crc32_le(0, (const unsigned char*)&cfg_copy, sizeof(pin_config_t) - sizeof(uint32_t));
    
    err = nvs_set_blob(handle, key, &cfg_copy, sizeof(pin_config_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save config for pin %d: %s", cfg->pin, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Saved config for pin %d: %s (%s)", 
                 cfg->pin, cfg->name, pin_type_to_string(cfg->type));
    }
    
    nvs_commit(handle);
    nvs_close(handle);
    return err;
}

// Загрузка конфигурации пина
esp_err_t config_load_pin(uint8_t pin, pin_config_t *cfg) {
    if (pin >= MAX_PINS || !cfg) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }
    
    char key[16];
    snprintf(key, sizeof(key), "pin_%d", pin);
    
    size_t size = sizeof(pin_config_t);
    err = nvs_get_blob(handle, key, cfg, &size);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        // Проверяем CRC
        uint32_t saved_crc = cfg->crc;
        cfg->crc = 0;
        uint32_t calc_crc = esp_rom_crc32_le(0, (const unsigned char*)cfg, sizeof(pin_config_t) - sizeof(uint32_t));
        cfg->crc = saved_crc;
        
        if (saved_crc != calc_crc) {
            ESP_LOGW(TAG, "CRC mismatch for pin %d, config corrupted", pin);
            return ESP_ERR_INVALID_CRC;
        }
        
        // Валидация типа
        if (cfg->type > PIN_TYPE_DISABLED || cfg->mode > PIN_MODE_ONEWIRE) {
            ESP_LOGW(TAG, "Invalid type/mode for pin %d, resetting", pin);
            return ESP_ERR_INVALID_ARG;
        }
        
        ESP_LOGD(TAG, "Loaded config for pin %d: %s (%s)", 
                 pin, cfg->name, pin_type_to_string(cfg->type));
        return ESP_OK;
    }
    
    return err;
}

// Удаление конфигурации пина
esp_err_t config_delete_pin(uint8_t pin) {
    if (pin >= MAX_PINS) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }
    
    char key[16];
    snprintf(key, sizeof(key), "pin_%d", pin);
    err = nvs_erase_key(handle, key);
    nvs_commit(handle);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Deleted config for pin %d", pin);
    }
    return err;
}

// Получение значения пина (заглушка)
float config_get_pin_value(uint8_t pin, bool is_output) {
    // Базовая реализация - возвращает 0
    return 0.0f;
}

// Преобразование типа в строку
const char* pin_type_to_string(pin_type_t type) {
    for (int i = 0; i < sizeof(type_map) / sizeof(type_map[0]); i++) {
        if (type_map[i].type == type) {
            return type_map[i].str;
        }
    }
    return "unknown";
}

// Преобразование строки в тип
pin_type_t pin_string_to_type(const char* str) {
    if (!str) return PIN_TYPE_DISABLED;
    for (int i = 0; i < sizeof(type_map) / sizeof(type_map[0]); i++) {
        if (strcmp(type_map[i].str, str) == 0) {
            return type_map[i].type;
        }
    }
    return PIN_TYPE_DISABLED;
}

// Преобразование режима в строку
const char* pin_mode_to_string(pin_mode_t mode) {
    for (int i = 0; i < sizeof(mode_map) / sizeof(mode_map[0]); i++) {
        if (mode_map[i].mode == mode) {
            return mode_map[i].str;
        }
    }
    return "digital";
}

// Преобразование строки в режим
pin_mode_t pin_string_to_mode(const char* str) {
    if (!str) return PIN_MODE_DIGITAL;
    for (int i = 0; i < sizeof(mode_map) / sizeof(mode_map[0]); i++) {
        if (strcmp(mode_map[i].str, str) == 0) {
            return mode_map[i].mode;
        }
    }
    return PIN_MODE_DIGITAL;
}

// Инициализация пина из конфигурации (применение)
esp_err_t config_apply_pin(const pin_config_t *cfg) {
    if (!cfg || !cfg->enabled || cfg->pin >= MAX_PINS) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Настраиваем GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << cfg->pin),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = cfg->pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    // Настраиваем режим в зависимости от типа
    if (cfg->type == PIN_TYPE_ACTUATOR) {
        io_conf.mode = GPIO_MODE_OUTPUT;
    } else if (cfg->type == PIN_TYPE_INPUT || cfg->type == PIN_TYPE_SENSOR) {
        io_conf.mode = GPIO_MODE_INPUT;
    }
    
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Applied config for pin %d: %s", cfg->pin, cfg->name);
    }
    return err;
}

// Загрузка всех конфигураций
esp_err_t config_load_all(pin_config_t *configs, uint8_t *count) {
    if (!configs || !count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *count = 0;
    nvs_handle_t handle;
    esp_err_t err = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;
    }
    
    nvs_iterator_t it = NULL;
    err = nvs_entry_find(NVS_DEFAULT_PART_NAME, CONFIG_NAMESPACE, NVS_TYPE_BLOB, &it);
    
    while (err == ESP_OK && *count < MAX_PINS) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        
        if (strncmp(info.key, "pin_", 4) == 0) {
            size_t size = sizeof(pin_config_t);
            err = nvs_get_blob(handle, info.key, &configs[*count], &size);
            if (err == ESP_OK) {
                (*count)++;
            }
        }
        err = nvs_entry_next(&it);
    }
    
    nvs_close(handle);
    ESP_LOGI(TAG, "Loaded %d pin configurations", *count);
    return ESP_OK;
}