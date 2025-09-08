#include<stdio.h>
#include<app_gw_wifi.h>
#include<app_gw_nvs.h>

void app_main(void) {
    app_nvs_init();
    app_nvs_load_all_flag();
    app_wifi_init();
}

