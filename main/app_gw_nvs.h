#pragma once
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void app_nvs_init();
void app_nvs_save_wifi_info(const char *ssid, const char *pass);
bool app_nvs_load_wifi_info(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
void app_nvs_earase_wifi_info();
void app_nvs_save_all_flag();
void app_nvs_load_all_flag();
void app_nvs_flag_change_state(uint8_t);
