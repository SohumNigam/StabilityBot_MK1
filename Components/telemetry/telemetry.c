#include "esp_http_server.h"
#include "esp_log.h"
#include "telemetry.h"
#include <stdio.h>

#define WS_TAG "WS"


extern float Kp;
extern float Ki;
extern float Kd;
extern float target_angle;
extern float total_error;

telemetry_t g_telemetry = {0};
SemaphoreHandle_t g_telemetry_mutex = NULL;


// Store connected WebSocket file descriptors
static int ws_fds[4] = {-1, -1, -1, -1};
static httpd_handle_t server_handle = NULL;


static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        int fd = httpd_req_to_sockfd(req);
        for (int i = 0; i < 1; i++) {
            if (ws_fds[i] == -1) { ws_fds[i] = fd; break; }
        }
        return ESP_OK;
    }

    // First call: get frame length, no payload buffer
    httpd_ws_frame_t frame = {
        .type    = HTTPD_WS_TYPE_TEXT,
        .payload = NULL,
        .len     = 0,
    };

    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK) {
        ESP_LOGE("WS", "Failed to get frame length: %s", esp_err_to_name(ret));
        return ret;
    }

    if (frame.len == 0) return ESP_OK; // empty frame, ignore

    // Second call: allocate exact size and read payload
    uint8_t *buf = calloc(1, frame.len + 1);
    if (!buf) return ESP_ERR_NO_MEM;

    frame.payload = buf;
    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK) {
        ESP_LOGE("WS", "Failed to read payload: %s", esp_err_to_name(ret));
        free(buf);
        return ret;
    }

    char *payload = (char *)buf;
    ESP_LOGI("WS", "Received: %s", payload);

    if (strstr(payload, "\"cmd\":\"pid\"")) {
        char *pp = strstr(payload, "\"p\":");
        char *ip = strstr(payload, "\"i\":");
        char *dp = strstr(payload, "\"d\":");
        if (pp) sscanf(pp + 4, "%f", &Kp);
        if (ip) sscanf(ip + 4, "%f", &Ki);
        if (dp) sscanf(dp + 4, "%f", &Kd);
        total_error = 0.0f;
        ESP_LOGI("WS", "PID → Kp=%.3f Ki=%.3f Kd=%.4f", Kp, Ki, Kd);
    }

    if (strstr(payload, "\"cmd\":\"target\"")) {
        char *ap = strstr(payload, "\"angle\":");
        if (ap) sscanf(ap + 8, "%f", &target_angle);
        ESP_LOGI("WS", "Target → %.2f°", target_angle);
    }

    free(buf);
    return ESP_OK;
}
// Call this from a periodic task to push telemetry
void ws_broadcast_telemetry(void) {
    if (!server_handle) return;

    telemetry_t t;
    if (xSemaphoreTake(g_telemetry_mutex, pdMS_TO_TICKS(5)) != pdTRUE) return;
    t = g_telemetry;  // copy under lock
    xSemaphoreGive(g_telemetry_mutex);

    char buf[256];
    int len = snprintf(buf, sizeof(buf),
    "{\"pitch\":%.2f,\"err\":%.2f,\"p\":%.2f,\"i\":%.2f,"
    "\"d\":%.2f,\"offset\":%.2f,\"speed\":%d,\"dt\":%.2f,"
    "\"kp\":%.4f,\"ki\":%.4f,\"kd\":%.4f,\"target\":%.2f}",
    t.pitch, t.err, t.p_term, t.i_term,
    t.d_term, t.total_offset, t.motor_speed, t.dt_ms,
    Kp, Ki, Kd, target_angle);

    httpd_ws_frame_t frame = {
        .type    = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)buf,
        .len     = len,
    };

    for (int i = 0; i < 4; i++) {
        if (ws_fds[i] != -1) {
            esp_err_t ret = httpd_ws_send_frame_async(server_handle, ws_fds[i], &frame);
            if (ret != ESP_OK) {
                ws_fds[i] = -1;  // client disconnected
            }
        }
    }
}


static esp_err_t root_handler(httpd_req_t *req) {
    FILE *f = fopen("/web/index.html", "r");
    if (!f) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "File not found");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    char buf[512];
    size_t bytes_read;
    while ((bytes_read = fread(buf, 1, sizeof(buf), f)) > 0) {
        httpd_resp_send_chunk(req, buf, bytes_read);
    }
    fclose(f);

    httpd_resp_send_chunk(req, NULL, 0); // terminate chunked response
    return ESP_OK;
}

void ws_server_start(void) {

    g_telemetry_mutex = xSemaphoreCreateMutex();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    ESP_ERROR_CHECK(httpd_start(&server_handle, &config));

    httpd_uri_t root = { .uri="/", .method=HTTP_GET, .handler=root_handler };
    httpd_uri_t ws   = {
        .uri       = "/ws",
        .method    = HTTP_GET,
        .handler   = ws_handler,
        .is_websocket = true
    };
    httpd_register_uri_handler(server_handle, &root);
    httpd_register_uri_handler(server_handle, &ws);
}