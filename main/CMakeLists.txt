idf_component_register(SRCS 
    "main.c"
    "src/core.c"
    "src/module.c"
    "src/config_manager.c"
    "src/config.c"
    "src/platform.c"
    "src/pin_config.c"
    "src/sensor_module.c"
    "src/ds18b20.c"
    "src/pcf8574.c"
    "src/wifi_manager.c"
    # "src/wifi_station.c"
    "src/pin_manager_module.c"
    "src/webserver/webserver_core.c"
    "src/webserver/webserver_handlers.c"
    "src/webserver/webserver_api.c"
    "src/webserver/webserver_config.c"
    "src/webserver/webserver_html.c"
    "src/modules/module_gpio.c"
    "src/modules/module_web.c"
    "src/modules/module_test.c"
    INCLUDE_DIRS 
    "include"
    REQUIRES 
        driver
        esp_event
        esp_http_server
        esp_netif
        esp_wifi
        lwip
        nvs_flash
        json                    # ← ДОБАВИТЬ (для cJSON)
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format -Wno-unused-variable)

target_include_directories(${COMPONENT_LIB} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)