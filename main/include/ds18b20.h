#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>   // <-- добавить для size_t

#ifdef __cplusplus
extern "C" {
#endif

// Инициализация 1-Wire шины
int ds18b20_init(int gpio_pin);

// Поиск всех датчиков на шине
int ds18b20_scan(uint64_t* rom_codes, int max_count);

// Чтение температуры с датчика по ROM коду
float ds18b20_read_temp(uint64_t rom_code);

// Чтение температуры по индексу (0 = первый найденный)
float ds18b20_read_temp_by_index(int index);

// Получить количество найденных датчиков
int ds18b20_get_count(void);

// Получить ROM код датчика по индексу
uint64_t ds18b20_get_rom(int index);

// Преобразовать ROM код в строку
void ds18b20_rom_to_string(uint64_t rom, char* buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif // DS18B20_H