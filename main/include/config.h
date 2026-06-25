#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char wifi_ssid[32];
    char wifi_password[32];
    char device_name[32];
    uint16_t web_port;
    bool enable_test_module;
} config_t;

void config_load(void);
void config_save(void);
config_t* config_get(void);

#endif