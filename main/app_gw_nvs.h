#pragma once
#include <stdio.h>
#include <string.h>

void app_nvs_init();
void app_nvs_save_wifi_info(const char *ssid, const char *pass);
bool app_nvs_load_wifi_info(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
void app_nvs_earase_wifi_info();
