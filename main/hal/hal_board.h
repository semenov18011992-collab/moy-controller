#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#include <stdbool.h>

typedef struct {
    void (*init_wifi)(void);
    void (*init_ethernet)(void);
    void (*init_gpio)(void);
    void (*init_i2c)(void);
    const char* board_name;
} board_hal_t;

extern board_hal_t board_hal;

void board_hal_detect(void);

#endif