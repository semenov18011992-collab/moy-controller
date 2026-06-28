idf_component_register(SRCS
    main.c
    src/sensor_module.c
    src/ds18b20.c
    src/wifi_manager.c
    src/config.c
    src/core.c
    src/module.c
    src/pcf8574.c
    src/platform.c
    src/webserver.c
    src/modules/module_gpio.c
    src/modules/module_web.c
    src/modules/module_test.c
    src/pin_manager_module.c  
    src/config_manager.c
    INCLUDE_DIRS include
    REQUIRES nvs_flash esp_netif esp_event esp_http_server driver lwip spi_flash freertos esp_driver_ledc json esp_wifi
    EMBED_FILES 
        data/index.html
        data/static/css/style.css
        data/static/js/app.js
        data/static/js/modules/api.js
        data/static/js/modules/dashboard.js
        data/static/js/modules/graphs.js
        data/static/js/modules/settings.js
        data/static/js/modules/ui.js
)