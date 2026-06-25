#ifndef MODULE_H
#define MODULE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MODULE_STATE_UNLOADED = 0,
    MODULE_STATE_LOADED,
    MODULE_STATE_INITIALIZED,
    MODULE_STATE_RUNNING,
    MODULE_STATE_ERROR
} module_state_t;

typedef struct {
    const char* name;
    const char* version;
    bool (*init)(void);
    bool (*start)(void);
    void (*update)(void);
    void (*stop)(void);
    module_state_t state;
    void* context;
} module_t;

bool module_register(module_t* mod);
bool module_init_all(void);
bool module_start_all(void);
void module_update_all(void);
void module_stop_all(void);
module_t* module_get(const char* name);
bool module_is_enabled(const char* name);
void module_register_builtins(void);

#endif