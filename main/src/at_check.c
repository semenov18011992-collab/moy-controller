#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define TAG "AT_CHECK"
#define UART_AT UART_NUM_1
#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_16

void at_check_init(void) {
    ESP_LOGI(TAG, "Checking ESP32-C6...");
    
    // Настройка UART для связи с C6
    uart_config_t uart_cfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(UART_AT, &uart_cfg);
    uart_set_pin(UART_AT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_AT, 256, 256, 0, NULL, 0);
    
    // Отправляем AT команду
    const char* cmd = "AT\r\n";
    uart_write_bytes(UART_AT, cmd, strlen(cmd));
    ESP_LOGI(TAG, "Sent: AT");
    
    // Ждём ответ (1 секунда)
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Читаем ответ
    uint8_t buf[128];
    int len = uart_read_bytes(UART_AT, buf, sizeof(buf) - 1, pdMS_TO_TICKS(500));
    if (len > 0) {
        buf[len] = '\0';
        ESP_LOGI(TAG, "Received: %s", buf);
        if (strstr((char*)buf, "OK") || strstr((char*)buf, "AT")) {
            ESP_LOGI(TAG, "✅ ESP32-C6 responds to AT commands!");
        } else {
            ESP_LOGI(TAG, "⚠️ ESP32-C6 responded but not with AT format");
        }
    } else {
        ESP_LOGE(TAG, "❌ No response from ESP32-C6! (Module may not be connected or not powered)");
    }
}