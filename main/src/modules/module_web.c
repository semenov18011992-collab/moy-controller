#include "module.h"
#include "logger.h"
#include "webserver.h"

static bool web_init(void) { 
    LOG_INFO("WEB", "Init"); 
    return true; 
}

static bool web_start(void) { 
    return webserver_start(); 
}

static void web_update(void) { 
    /* nothing */ 
}

static void web_stop(void) { 
    webserver_stop(); 
}

module_t module_web = {
    .name = "web",
    .version = "1.0.0",
    .init = web_init,
    .start = web_start,
    .update = web_update,
    .stop = web_stop
};