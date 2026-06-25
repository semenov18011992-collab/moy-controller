#include "hal_board.h"
#include "esp_chip_info.h"

extern board_hal_t hal_board_esp32;
extern board_hal_t hal_board_esp32c6;
extern board_hal_t hal_board_esp32p4;
extern board_hal_t hal_board_default;

board_hal_t board_hal;

void board_hal_detect(void) {
    esp_chip_info_t info;
    esp_chip_info(&info);
    
    switch (info.model) {
        case CHIP_ESP32:
            board_hal = hal_board_esp32;
            break;
        case CHIP_ESP32C6:
            board_hal = hal_board_esp32c6;
            break;
        case CHIP_ESP32P4:
            board_hal = hal_board_esp32p4;
            break;
        default:
            board_hal = hal_board_default;
            break;
    }
    ESP_LOGI("HAL", "Detected board: %s", board_hal.board_name);
}