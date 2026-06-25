#ifndef PCF8574_H
#define PCF8574_H

#include <stdint.h>
#include <stdbool.h>

bool pcf8574_probe(uint8_t addr);
bool pcf8574_write(uint8_t addr, uint8_t data);
uint8_t pcf8574_read(uint8_t addr);

#endif