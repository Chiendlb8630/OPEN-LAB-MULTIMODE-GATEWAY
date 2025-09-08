#include <string.h>
#include "app_gw_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_http_server.h"

#define WIFI_AP_SSID "IOT Gateway Wifi"
#define WIFI_AP_PASS "12345678"

static const char *TAG = "WIFI_MODULE";
// ================================================ Web page HTML form ===================================================
static const char html_form[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<style>"
"body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; margin:0; padding:20px; }"
"div.container { max-width: 400px; margin:auto; background:#fff; padding:20px; border-radius:12px; box-shadow:0 4px 8px rgba(0,0,0,0.1); }"
"h2 { color:#333; margin-bottom:20px; }"
"input[type=text], input[type=password] { width:90%; padding:12px; margin:8px 0; border:1px solid #ccc; border-radius:8px; font-size:16px; }"
"input[type=submit] { background:#007BFF; color:white; padding:14px; border:none; border-radius:8px; cursor:pointer; font-size:16px; width:95%; }"
"input[type=submit]:hover { background:#0056b3; }"
"</style>"
"</head>"
"<body>"
"<div class=\"container\">"
"<h2>ESP32 WiFi Setup</h2>"
"<form action=\"/connect\" method=\"post\">"
"<input type=\"text\" name=\"ssid\" placeholder=\"WiFi SSID\" required><br>"
"<input type=\"password\" name=\"password\" placeholder=\"WiFi Password\" required><br>"
"<input type=\"submit\" value=\"Save\">"
"</form>"
"</div>"
"</body>"
"</html>";
// ==============================================================================================================================
wifi_info_t g_wifi_info;
int g_wifi_status = 0;

void app_wifi_switch_to_sta(const char *ssid_rcvAP, const char *pass_rcvAP);

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, html_form, strlen(html_form));
    return ESP_OK;
}

static esp_err_t connect_post_handler(httpd_req_t *req) {
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0';
    ESP_LOGI(TAG, "Recv: %s", buf);
    char ssid[32] = {0}, pass[64] = {0};
    sscanf(buf, "ssid=%31[^&]&password=%63s", ssid, pass);

    app_wifi_switch_to_sta(ssid, pass);
    ESP_LOGI(TAG, "SSID: %s, PASS: %s", ssid, pass);
    httpd_resp_sendstr(req, "Received! ESP switch to STA");
    return ESP_OK;
}

httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};

httpd_uri_t connect_uri = {
    .uri = "/connect",
    .method = HTTP_POST,
    .handler = connect_post_handler,
    .user_ctx = NULL
};


void start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &connect_uri);
    }
}

static void wifi_init_softap(void) {
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = 1,
            .password = WIFI_AP_PASS,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_AP_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "AP started! SSID:%s, PASS:%s", WIFI_AP_SSID, WIFI_AP_PASS);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void app_wifi_switch_to_sta(const char *ssid_rcvAP, const char *pass_rcvAP)
{
    ESP_ERROR_CHECK(esp_wifi_stop());   

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid_rcvAP, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass_rcvAP, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void app_wifi_init(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    if (load_wifi_credentials(g_wifi_info.ssid, sizeof(g_wifi_info.ssid), g_wifi_info.pass, sizeof(g_wifi_info.pass))) {
        ESP_LOGI(TAG, "Found WiFi credentials in NVS, SSID: %s", g_wifi_info.ssid);
        app_wifi_switch_to_sta(g_wifi_info.ssid, g_wifi_info.pass);
    } else {
        ESP_LOGI(TAG, "No WiFi credentials, starting AP mode");
        wifi_init_softap();
        start_webserver();
    }
}
