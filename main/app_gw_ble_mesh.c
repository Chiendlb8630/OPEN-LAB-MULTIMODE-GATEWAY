#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "ble_mesh_example_init.h"

#define BLE_MESH_TAG "BLE_MESH_MODULE"

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
     .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 10),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(1, 10),
};

static esp_ble_mesh_client_t onoff_client1;
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_cli_pub1, 1, ROLE_NODE);
ESP_BLE_MESH_MODEL_PUB_DEFINE(pub_goos_1, 1, ROLE_NODE);

static esp_ble_mesh_model_op_t vnd_op_config[] = {
    ESP_BLE_MESH_MODEL_OP(APP_GW_GET_NODE_INFO, 1),
    ESP_BLE_MESH_MODEL_OP_END
};

esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, GATEWAY_GET_NODE_INFO,
    vnd_op_config, NULL, NULL),
};

