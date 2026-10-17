#include "ota.h"
#include "ota_webpage.h"

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* OPÇÃO DE ATIVAÇÃO DO LED D2: Define para 1 para piscar por pacote, 0 para desativar */
#define ENABLE_OTA_LED_BLINK 1

#if ENABLE_OTA_LED_BLINK
#define BLINK_GPIO GPIO_NUM_2  // LED integrado D2 no GPIO 2
#endif

static const char *TAG = "OTA_WEB";

/* Handler do caminho raiz (/): responde apenas com "OK" em texto simples */
static esp_err_t index_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK", 2);
    return ESP_OK;
}

/* Handler do caminho /update (GET): serve a página HTML de upload */
static esp_err_t update_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, ota_upload_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Handler do caminho /update (POST): recebe o ficheiro .bin e grava na Flash */
static esp_err_t ota_post_handler(httpd_req_t *req) {
    esp_ota_handle_t ota_handle;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    if (update_partition == NULL) {
        ESP_LOGE(TAG, "No available OTA partition");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

#if ENABLE_OTA_LED_BLINK
    // Configura o pino do LED D2 como saída
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
#endif

    ESP_LOGI(TAG, "Starting write to OTA partition: %s", update_partition->label);
    esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start OTA (%s)", esp_err_to_name(err));
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char buf[1024];
    int remaining = req->content_len;
    int received;

#if ENABLE_OTA_LED_BLINK
    int led_state = 0;
#endif

    while (remaining > 0) {
        if ((received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Error receiving data from socket");
#if ENABLE_OTA_LED_BLINK
            gpio_set_level(BLINK_GPIO, 0); // Desliga o LED em caso de erro
#endif
            esp_ota_abort(ota_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

#if ENABLE_OTA_LED_BLINK
        // Alterna o estado do LED D2 a cada pacote recebido (versão original)
        led_state = !led_state;
        gpio_set_level(BLINK_GPIO, led_state);
#endif

        err = esp_ota_write(ota_handle, (const void *)buf, received);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error writing to flash (%s)", esp_err_to_name(err));
#if ENABLE_OTA_LED_BLINK
            gpio_set_level(BLINK_GPIO, 0); // Desliga o LED em caso de erro
#endif
            esp_ota_abort(ota_handle);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        remaining -= received;
    }

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to validate OTA image (%s)", esp_err_to_name(err));
#if ENABLE_OTA_LED_BLINK
        gpio_set_level(BLINK_GPIO, 0);
#endif
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set new boot partition (%s)", esp_err_to_name(err));
#if ENABLE_OTA_LED_BLINK
        gpio_set_level(BLINK_GPIO, 0);
#endif
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

#if ENABLE_OTA_LED_BLINK
    // Mantém o LED aceso fixo para indicar conclusão bem-sucedida
    gpio_set_level(BLINK_GPIO, 1);
#endif

    ESP_LOGI(TAG, "Update completed! Rebooting...");
    httpd_resp_sendstr(req, "Success! The ESP32 is restarting with the new firmware...");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
#if ENABLE_OTA_LED_BLINK
    gpio_set_level(BLINK_GPIO, 0);
#endif
    esp_restart();

    return ESP_OK;
}

/* Arranque do Servidor Web HTTP */
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        // Rota GET para o caminho raiz (/) -> Devolve apenas "OK"
        httpd_uri_t index_uri = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = index_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        // Rota GET para /update -> Devolve a interface de upload
        httpd_uri_t update_get_uri = {
            .uri      = "/update",
            .method   = HTTP_GET,
            .handler  = update_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &update_get_uri);

        // Rota POST para /update -> Recebe o ficheiro e faz flash
        httpd_uri_t update_post_uri = {
            .uri      = "/update",
            .method   = HTTP_POST,
            .handler  = ota_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &update_post_uri);
    }
    return server;
}

/* Inicialização do Wi-Fi em modo SoftAP */
void wifi_init_softap(const char *ssid, const char *password, int max_conn) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.ssid_len = strlen(ssid);
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = max_conn;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    if (strlen(password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi SoftAP initialized. SSID:%s password:%s", ssid, password);
}

/* Event Handler simplificado apenas para ligações Wi-Fi */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Connection lost. Trying to reconnect...");
        esp_wifi_connect();
    }
}

/* Inicialização do Wi-Fi em modo Station (Liga ao Router) */
void wifi_init_sta(const char *ssid, const char *password) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi network: %s ...", ssid);
}

/* Função para obter o IP atual e copiar para uma string */
esp_err_t ota_get_ip(char *ip_str) {
    if (ip_str == NULL) return ESP_ERR_INVALID_ARG;

    esp_netif_ip_info_t ip_info;
    
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    
    if (netif == NULL || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK || ip_info.ip.addr == 0) {
        netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        if (netif == NULL || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
            sprintf(ip_str, "0.0.0.0");
            return ESP_FAIL;
        }
    }

    snprintf(ip_str, 16, IPSTR, IP2STR(&ip_info.ip));
    return ESP_OK;
}