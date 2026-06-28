#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool wifi_manager_init(void);
bool wifi_manager_is_connected(void);

// ← ДОБАВИТЬ ЭТУ СТРОКУ
void wifi_init_sta(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H