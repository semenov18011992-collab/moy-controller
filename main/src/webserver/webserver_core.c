#include "webserver.h"
#include "webserver_priv.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define TAG "WEBSERVER"

// ============================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================
static httpd_handle_t server = NULL;
static bool is_running = false;

// ============================================
// ЗАПУСК СЕРВЕРА (ОСНОВНАЯ ФУНКЦИЯ)
// ============================================
bool webserver_start(void) {
    if (is_running) {
        ESP_LOGW(TAG, "Server already running");
        return true;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.stack_size = 12288;   // 12KB для надёжности
    config.task_priority = 5;
    
    ESP_LOGI(TAG, "Starting webserver...");
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to start server");
        return false;
    }
    
    // Регистрируем все обработчики
    register_handlers(server);
    
    is_running = true;
    ESP_LOGI(TAG, "✅ Webserver started on port 80");
    return true;
}

// ============================================
// ОСТАНОВКА СЕРВЕРА
// ============================================
void webserver_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        is_running = false;
        ESP_LOGI(TAG, "Webserver stopped");
    }
}

// ============================================
// ПРОВЕРКА СТАТУСА
// ============================================
bool webserver_is_running(void) {
    return is_running && server != NULL;
}