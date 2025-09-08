#include "nvs_flash.h"
#include "nvs.h"

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
    if (err != ESP_OK) return false;
    err = nvs_get_str(nvs, "wifi_ssid", ssid, &ssid_len);
    if (err != ESP_OK) { nvs_close(nvs); return false; }
    err = nvs_get_str(nvs, "wifi_pass", pass, &pass_len);
    if (err != ESP_OK) { nvs_close(nvs); return false; }
    nvs_close(nvs);
    return true;
}

void app_nvs_earase_wifi_info(){
    nvs_handle_t nvs;
    nvs_erase_key(nvs, "wifi_ssid");
    nvs_erase_key(nvs, "wifi_pass");
    nvs_commit(nvs);
}
