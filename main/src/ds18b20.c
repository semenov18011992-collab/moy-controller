#include "ds18b20.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include <string.h>
#include <stdlib.h>

#define TAG "DS18B20"

#define DS18B20_CMD_CONVERT_TEMP      0x44
#define DS18B20_CMD_READ_SCRATCHPAD   0xBE
#define DS18B20_CMD_SKIP_ROM          0xCC
#define DS18B20_CMD_SEARCH_ROM        0xF0

#define ONE_WIRE_DELAY_A             6
#define ONE_WIRE_DELAY_B             64
#define ONE_WIRE_DELAY_C             60
#define ONE_WIRE_DELAY_D             10
#define ONE_WIRE_DELAY_E             9
#define ONE_WIRE_DELAY_F             55
#define ONE_WIRE_DELAY_H             480
#define ONE_WIRE_DELAY_I             70
#define ONE_WIRE_DELAY_J             410

static int gpio_pin = -1;
static uint64_t* rom_codes = NULL;
static int rom_count = 0;
static int max_rom = 0;

static void onewire_set_output(bool level) {
    gpio_set_level(gpio_pin, level ? 1 : 0);
    if (level) {
        gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    } else {
        gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT_OD);
    }
}

static void onewire_set_input(void) {
    gpio_set_direction(gpio_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(gpio_pin, GPIO_PULLUP_ONLY);
}

static bool onewire_read_bit(void) {
    onewire_set_output(0);
    esp_rom_delay_us(ONE_WIRE_DELAY_A);
    onewire_set_input();
    esp_rom_delay_us(ONE_WIRE_DELAY_E);
    bool bit = gpio_get_level(gpio_pin);
    esp_rom_delay_us(ONE_WIRE_DELAY_F);
    return bit;
}

static void onewire_write_bit(bool bit) {
    onewire_set_output(0);
    if (bit) {
        esp_rom_delay_us(ONE_WIRE_DELAY_A);
        onewire_set_input();
        esp_rom_delay_us(ONE_WIRE_DELAY_B);
    } else {
        esp_rom_delay_us(ONE_WIRE_DELAY_C);
        onewire_set_input();
        esp_rom_delay_us(ONE_WIRE_DELAY_D);
    }
}

static bool onewire_reset(void) {
    onewire_set_output(0);
    esp_rom_delay_us(ONE_WIRE_DELAY_H);
    onewire_set_input();
    esp_rom_delay_us(ONE_WIRE_DELAY_I);
    bool presence = !gpio_get_level(gpio_pin);
    esp_rom_delay_us(ONE_WIRE_DELAY_J);
    return presence;
}

static void onewire_write_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        onewire_write_bit(data & 1);
        data >>= 1;
    }
}

static uint8_t onewire_read_byte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data >>= 1;
        if (onewire_read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

static bool onewire_search(uint64_t* rom, int* last_discrepancy, int* last_family, int* last_device) {
    int discrepancy = 0;
    uint8_t rom_byte = 0;
    int rom_bit_index = 0;
    
    if (!onewire_reset()) return false;
    
    onewire_write_byte(DS18B20_CMD_SEARCH_ROM);
    
    for (int byte_index = 0; byte_index < 8; byte_index++) {
        rom_byte = 0;
        for (int bit_index = 0; bit_index < 8; bit_index++) {
            bool bit = onewire_read_bit();
            bool comp_bit = onewire_read_bit();
            
            if (bit && comp_bit) return false;
            
            bool bit_value = bit;
            if (!bit && !comp_bit) {
                if (rom_bit_index == *last_discrepancy) {
                    bit_value = 1;
                } else if (rom_bit_index > *last_discrepancy) {
                    bit_value = 0;
                } else {
                    bit_value = 0;
                }
                if (bit_value) discrepancy = rom_bit_index;
            } else {
                if (rom_bit_index == *last_discrepancy) discrepancy = -1;
            }
            
            onewire_write_bit(bit_value);
            rom_byte >>= 1;
            if (bit_value) rom_byte |= 0x80;
            rom_bit_index++;
        }
        ((uint8_t*)rom)[byte_index] = rom_byte;
    }
    
    if (discrepancy == -1) {
        *last_discrepancy = -1;
        return true;
    }
    
    *last_discrepancy = discrepancy;
    return true;
}

// ============ ИСПРАВЛЕННАЯ ФУНКЦИЯ (возвращает int) ============
int ds18b20_init(int gpio_pin_num) {
    if (gpio_pin_num < 0) return 0;
    
    gpio_pin = gpio_pin_num;
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    if (!onewire_reset()) {
        ESP_LOGW(TAG, "No DS18B20 sensors found on GPIO %d", gpio_pin);
        return 0;
    }
    
    max_rom = 16;
    rom_codes = malloc(max_rom * sizeof(uint64_t));
    if (!rom_codes) return 0;
    
    rom_count = 0;
    int last_discrepancy = -1;
    int last_family = 0;
    int last_device = 0;
    uint64_t rom;
    
    while (rom_count < max_rom) {
        if (!onewire_search(&rom, &last_discrepancy, &last_family, &last_device)) {
            break;
        }
        if (last_discrepancy == -1) break;
        rom_codes[rom_count++] = rom;
    }
    
    ESP_LOGI(TAG, "Found %d DS18B20 sensors on GPIO %d", rom_count, gpio_pin);
    return rom_count;
}

int ds18b20_scan(uint64_t* roms, int max_count) {
    int count = 0;
    for (int i = 0; i < rom_count && i < max_count; i++) {
        roms[i] = rom_codes[i];
        count++;
    }
    return count;
}

float ds18b20_read_temp(uint64_t rom_code) {
    if (!onewire_reset()) return -999.0f;
    
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    onewire_write_byte(DS18B20_CMD_CONVERT_TEMP);
    
    esp_rom_delay_us(750000);
    
    if (!onewire_reset()) return -999.0f;
    
    onewire_write_byte(DS18B20_CMD_SKIP_ROM);
    onewire_write_byte(DS18B20_CMD_READ_SCRATCHPAD);
    
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = onewire_read_byte();
    }
    
    int16_t temp_raw = (data[1] << 8) | data[0];
    float temp = temp_raw / 16.0f;
    
    return temp;
}

float ds18b20_read_temp_by_index(int index) {
    if (index < 0 || index >= rom_count) return -999.0f;
    return ds18b20_read_temp(rom_codes[index]);
}

int ds18b20_get_count(void) {
    return rom_count;
}

uint64_t ds18b20_get_rom(int index) {
    if (index < 0 || index >= rom_count) return 0;
    return rom_codes[index];
}

void ds18b20_rom_to_string(uint64_t rom, char* buf, size_t len) {
    snprintf(buf, len, "%016llX", (unsigned long long)rom);
}