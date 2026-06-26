idf_component_register(SRCS
    main.c
    src/sensor_module.c
    src/config.c
    src/core.c
    src/module.c
    src/pcf8574.c
    src/platform.c
    src/webserver.c
    src/modules/module_gpio.c
    src/modules/module_web.c
    src/modules/module_test.c
    INCLUDE_DIRS include
    REQUIRES nvs_flash esp_netif esp_event esp_http_server driver lwip spi_flash freertos esp_driver_ledc json
    EMBED_FILES
        ../data/index.html
        ../data/css/style.css
        ../data/js/app.js
)