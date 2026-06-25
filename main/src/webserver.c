#include "webserver.h"
#include "logger.h"
#include <esp_http_server.h>
// #include <esp_spiffs.h>
#include <string.h>
#include <sys/stat.h>

static httpd_handle_t server = NULL;

// Простая функция определения MIME-типа по расширению
static const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".ico") == 0) return "image/x-icon";
    return "text/plain";
}

static esp_err_t send_file(httpd_req_t* req, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    if (!buf) {
        fclose(f);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    fread(buf, 1, len, f);
    fclose(f);
    buf[len] = 0;
    httpd_resp_set_type(req, get_mime_type(path));
    httpd_resp_send(req, buf, len);
    free(buf);
    return ESP_OK;
}

static esp_err_t root_handler(httpd_req_t* req) {
    return send_file(req, "/spiffs/index.html");
}

static esp_err_t api_status_handler(httpd_req_t* req) {
    const char* response = "{\"status\":\"ok\",\"version\":\"v6.0\",\"modules\":3}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

bool webserver_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    if (httpd_start(&server, &config) != ESP_OK) {
        LOG_ERROR("WEBSERVER", "Failed to start");
        return false;
    }
    httpd_uri_t uri_root = { .uri = "/", .method = HTTP_GET, .handler = root_handler };
    httpd_register_uri_handler(server, &uri_root);
    httpd_uri_t uri_api = { .uri = "/api/status", .method = HTTP_GET, .handler = api_status_handler };
    httpd_register_uri_handler(server, &uri_api);
    LOG_INFO("WEBSERVER", "Started on port %d", config.server_port);
    return true;
}

void webserver_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}