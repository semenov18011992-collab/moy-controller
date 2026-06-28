#include "webserver_priv.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define TAG "WEBSERVER_HANDLERS"

// ============================================
// РЕГИСТРАЦИЯ ВСЕХ ОБРАБОТЧИКОВ
// ============================================
void register_handlers(httpd_handle_t server) {
    if (!server) {
        ESP_LOGE(TAG, "Server handle is NULL");
        return;
    }
    
    // === СТАТИЧЕСКИЙ HTML ===
    httpd_uri_t uri_root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &uri_root);
    
    // === ОСНОВНЫЕ API ===
    httpd_uri_t handlers[] = {
        // Получение всех пинов
        {.uri = "/api/pins", .method = HTTP_GET, .handler = api_pins_handler, .user_ctx = NULL},
        // История пина
        {.uri = "/api/history", .method = HTTP_GET, .handler = api_history_handler, .user_ctx = NULL},
        // Управление пином
        {.uri = "/api/pin/set", .method = HTTP_GET, .handler = api_set_pin_handler, .user_ctx = NULL},
        // Конфигурация пина
        {.uri = "/api/pin/config", .method = HTTP_POST, .handler = pin_config_handler, .user_ctx = NULL},
        // Все конфигурации
        {.uri = "/api/pins/config/all", .method = HTTP_GET, .handler = pins_config_all_handler, .user_ctx = NULL},
        // Удаление конфигурации
        {.uri = "/api/pin/delete", .method = HTTP_DELETE, .handler = pin_delete_handler, .user_ctx = NULL},
        // Системная информация
        {.uri = "/api/system", .method = HTTP_GET, .handler = system_info_handler, .user_ctx = NULL},
        {.uri = NULL, .method = 0, .handler = NULL, .user_ctx = NULL}
    };
    
    for (int i = 0; handlers[i].uri != NULL; i++) {
        if (httpd_register_uri_handler(server, &handlers[i]) != ESP_OK) {
            ESP_LOGW(TAG, "Failed to register handler for %s", handlers[i].uri);
        } else {
            ESP_LOGD(TAG, "Registered handler: %s", handlers[i].uri);
        }
    }
    
    ESP_LOGI(TAG, "All handlers registered");
}