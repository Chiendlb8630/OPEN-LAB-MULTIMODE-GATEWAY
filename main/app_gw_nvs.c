#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "app_gw_wifi.h"

void app_nvs_init(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_nvs_save_wifi_info(const char *ssid, const char *pass) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "wifi_ssid", ssid));
    ESP_ERROR_CHECK(nvs_set_str(nvs, "wifi_pass", pass));
    ESP_ERROR_CHECK(nvs_commit(nvs));
    nvs_close(nvs);
}

bool app_nvs_load_wifi_info(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS namespace (err=0x%x)", err);
        return false;
    }
    err = nvs_get_str(nvs, "wifi_ssid", ssid, &ssid_len);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to read SSID (err=0x%x)", err);
        nvs_close(nvs);
        return false;
    }
    err = nvs_get_str(nvs, "wifi_pass", pass, &pass_len);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to read PASS (err=0x%x)", err);
        nvs_close(nvs);
        return false;
    }
    nvs_close(nvs);
    ESP_LOGI("NVS", "Loaded WiFi SSID: %s", ssid);
    ESP_LOGI("NVS", "Loaded WiFi PASS: %s", pass);
    return true;
}

void app_nvs_save_all_flag(void) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS (err=0x%x)", err);
        return;
    }
    err = nvs_set_u8(nvs, "wifi_sta_flag", g_wifi_sta_flag);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to save wifi_sta_flag (err=0x%x)", err);
        nvs_close(nvs);
        return;
    }
    err = nvs_commit(nvs);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Commit failed (err=0x%x)", err);
    } else {
        ESP_LOGI("NVS", "Saved flags: STA=%d", (int)g_wifi_sta_flag);
    }
    nvs_close(nvs);
}

void app_nvs_load_all_flag(void) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS (err=0x%x)", err);
        return;
    }

    err = nvs_get_u8(nvs, "wifi_sta_flag", &g_wifi_sta_flag);
    if (err != ESP_OK) {
        ESP_LOGW("NVS", "No STA flag found, default=0");
        g_wifi_sta_flag = 0;
    }
    ESP_LOGI("NVS", "Loaded flags: STA=%d ", (int)g_wifi_sta_flag);
    nvs_close(nvs);
}

void app_nvs_earase_wifi_info(){
    nvs_handle_t nvs = NULL;
    nvs_erase_key(nvs, "wifi_ssid");
    nvs_erase_key(nvs, "wifi_pass");
    nvs_commit(nvs);
}

void app_nvs_flag_change_state(uint8_t state){
    g_wifi_sta_flag = state;
    app_nvs_save_all_flag();
}