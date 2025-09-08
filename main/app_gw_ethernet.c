
// #include "ethernet_init.h"
// // #include "esp_log.h"
// #include "esp_check.h"
// #include "esp_mac.h"
// #include "driver/gpio.h"
// #include "sdkconfig.h"
// #include "driver/spi_master.h"

// static const char *TAG = "ETHERNET_MODULE";
// static bool gpio_isr_svc_init_by_eth = false; 

// static esp_err_t spi_bus_init(void)
// {
//     esp_err_t ret = ESP_OK;
//     ret = gpio_install_isr_service(0);
//     if (ret == ESP_OK) {
//         gpio_isr_svc_init_by_eth = true;
//     } else if (ret == ESP_ERR_INVALID_STATE) {
//         ESP_LOGW(TAG, "GPIO SPI ISR handler has been already installed");
//         ret = ESP_OK; 
//     } else {
//         ESP_LOGE(TAG, "GPIO ISR handler install failed");
//         goto err;
//     }

//     spi_bus_config_t buscfg = {
//         .miso_io_num = 19 ,
//         .mosi_io_num = 23,
//         .sclk_io_num = 18 ,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//     };
//     ESP_GOTO_ON_ERROR(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO), err, TAG, "SPI host 3 init failed");

// err:
//     return ret;
// }

// static esp_eth_handle_t app_ethernet_init_w5500(const uint8_t *mac_addr)
// {
//     esp_eth_handle_t eth_handle = NULL;
//     esp_eth_mac_t *mac = NULL;
//     esp_eth_phy_t *phy = NULL;

//     eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
//     eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
//     phy_config.phy_addr = 0;   
//     phy_config.reset_gpio_num = 2;

//     spi_device_interface_config_t spi_devcfg = {
//         .mode = 0,
//         .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
//         .queue_size = 20,
//         .spics_io_num = 10,
//     };
//     eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI3_HOST, &spi_devcfg);
//     w5500_config.int_gpio_num = 4;
//     mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
//     phy = esp_eth_phy_new_w5500(&phy_config);
//     esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
//     // ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
//     esp_err_t ret = esp_eth_driver_install(&config, &eth_handle);
//     if (ret != ESP_OK) {
//     ESP_LOGE(TAG, "Ethernet driver install failed (no W5500 connected?)");
//     return ret;   
//     }
    
//     if (mac_addr != NULL) {
//         ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, (void *)mac_addr));
//     }
//     return eth_handle;
// }

// esp_err_t app_ethernet_driver_init(esp_eth_handle_t *eth_handles_out)
// {
//     esp_err_t ret = ESP_OK; 
//     ESP_GOTO_ON_FALSE(eth_handles_out != NULL, ESP_ERR_INVALID_ARG,  err, TAG, "invalid ethernet spi driver arguments");
//     ESP_GOTO_ON_ERROR(spi_bus_init(), err, TAG, "SPI bus init failed");
//     uint8_t base_mac_addr[6];
//     ESP_GOTO_ON_ERROR(esp_efuse_mac_get_default(base_mac_addr), err, TAG, "get EFUSE MAC failed");
//     static uint8_t local_mac[6];
//     esp_derive_local_mac(local_mac, base_mac_addr);
//     *eth_handles_out = eth_init_w5500(local_mac);
//     return ESP_OK;

// err:   
//     return ret;
// }

// esp_err_t app_ethernet_driver_deinit(esp_eth_handle_t eth_handle)
// {
//     esp_err_t ret = ESP_OK; 
//     ESP_RETURN_ON_FALSE(eth_handle != NULL, ESP_ERR_INVALID_ARG, TAG, "Ethernet handle cannot be NULL");
//     esp_eth_mac_t *mac = NULL;
//     esp_eth_phy_t *phy = NULL;
//     ESP_ERROR_CHECK(esp_eth_get_mac_instance(eth_handle, &mac));
//     ESP_ERROR_CHECK(esp_eth_get_phy_instance(eth_handle, &phy));

//     ESP_RETURN_ON_ERROR(esp_eth_driver_uninstall(eth_handle), TAG, "Ethernet uninstall failed");

//     if (mac != NULL) {
//         mac->del(mac);
//     }
//     if (phy != NULL) {
//         phy->del(phy);
//     }
//     ESP_ERROR_CHECK(spi_bus_free(SPI3_HOST));

//     if (gpio_isr_svc_init_by_eth) {
//         ESP_LOGW(TAG, "uninstalling GPIO ISR service!");
//         gpio_uninstall_isr_service();
//         gpio_isr_svc_init_by_eth = false;
//     }
//     return ESP_OK;
// }

// static void app_ethernet_ethernet_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
// {
//     uint8_t mac_addr[6] = {0};
//     esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

//     switch (event_id) {
//     case ETHERNET_EVENT_CONNECTED:
//         esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
//         ESP_LOGI(TAG, "Ethernet Link Up");
//         ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
//                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
//         break;
//     case ETHERNET_EVENT_DISCONNECTED:
//         ESP_LOGI(TAG, "Ethernet Link Down");
//         break;
//     case ETHERNET_EVENT_START:
//         ESP_LOGI(TAG, "Ethernet Started");
//         break;
//     case ETHERNET_EVENT_STOP:
//         ESP_LOGI(TAG, "Ethernet Stopped");
//         break;
//     default:
//         break;
//     }
// }

// static void app_ethernet_got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
// {
//     ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
//     const esp_netif_ip_info_t *ip_info = &event->ip_info;
//     ESP_LOGI(TAG, "Ethernet Got IP Address");
//     ESP_LOGI(TAG, "~~~~~~~~~~~");
//     ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
//     ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
//     ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
//     ESP_LOGI(TAG, "~~~~~~~~~~~");
// }

// void app_ethernet_init(void)
// {
//     esp_eth_handle_t eth_handle;
//     ESP_ERROR_CHECK(app_ethernet_driver_init(&eth_handle));

//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
//     esp_netif_t *eth_netif = esp_netif_new(&cfg);

//     esp_eth_netif_glue_handle_t eth_netif_glue = esp_eth_new_netif_glue(eth_handle);
//     ESP_ERROR_CHECK(esp_netif_attach(eth_netif, eth_netif_glue));

//     ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &app_ethernet_ethernet_event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &app_ethernet_got_ip_event_handler, NULL));
//     ESP_ERROR_CHECK(esp_eth_start(eth_handle));
// }
