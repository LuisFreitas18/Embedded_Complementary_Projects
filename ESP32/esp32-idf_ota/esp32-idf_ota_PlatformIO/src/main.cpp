#include "ota.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_desc.h"

#define AP_ESP_WIFI_SSID      "ESP32_OTA_AP"
#define AP_ESP_WIFI_PASS      "12345678"
#define MAX_STA_CONN       4

#define WIFI_SSID      "example_ssid"
#define WIFI_PASS      "example_password"

static const char *TAG = "OTA_WEB";

extern "C" void app_main(void) {
    // 1. Inicialização do NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Validação de Rollback Automático (Anti-Brick)
    // Se este arranque ocorreu após uma atualização OTA, confirma que a nova versão é válida
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "New firmware updated successfully! Marking as valid...");
            esp_ota_mark_app_valid_cancel_rollback();
        }
    }

    /*
    wifi_init_softap(AP_ESP_WIFI_SSID, AP_ESP_WIFI_PASS, MAX_STA_CONN);

    start_webserver();
    ESP_LOGI(TAG, "Web server initialized! Access at http://192.168.4.1/");
    */

    // 2. Inicialização do Wi-Fi em modo STA
    wifi_init_sta(WIFI_SSID, WIFI_PASS);

    start_webserver();
    
    // Arrancar o servidor OTA
    start_webserver();

    // Aguarda 3 segundos para garantir que o DHCP atribuiu o IP
    vTaskDelay(pdMS_TO_TICKS(3000));

    char my_ip[16];
    if (ota_get_ip(my_ip) == ESP_OK) {
        ESP_LOGI(TAG, ">>> My IP is: %s <<<", my_ip);
    } else {
        ESP_LOGW(TAG, "Still waiting for connection/IP...");
    }
}