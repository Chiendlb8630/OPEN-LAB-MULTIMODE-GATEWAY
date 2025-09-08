
#define WIFI_AP_SSID "IOT Gateway Wifi"
#define WIFI_AP_PASS "12345678"

typedef struct {
    char ssid[32];
    char pass[64];
}wifi_info_t;

extern wifi_info_t g_wifi_info;
extern int g_wifi_status;
