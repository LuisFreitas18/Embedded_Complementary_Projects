#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_server.h"

#include "../ap_config.h"
#include "../web_page.h"

#include "../config.h"
#include "../hal_mcu.h"


static httpd_handle_t server = NULL;
static const char *TAG = "ESP32_MATRIZ";
int matrix[ROWS][COLS] = {0};

// --- Imprimir Matriz no Terminal ---
void print_matrix(void) {
    ESP_LOGI(TAG, "--- Estado Atual da Matriz ---");
    for (int r = 0; r < ROWS; r++) {
        printf("[ ");
        for (int c = 0; c < COLS; c++) {
            printf("%d\t", matrix[r][c]);
        }
        printf("]\n");
    }
    printf("-----------------------------\n");
}

// --- Gestão de NVS (Memória Flash) ---
void save_cell_to_nvs(int r, int c, int val) {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READWRITE, &my_handle) == ESP_OK) {
        char key[15];
        snprintf(key, sizeof(key), "cell_%d_%d", r, c);
        nvs_set_i32(my_handle, key, val);
        nvs_commit(my_handle);
        nvs_close(my_handle);
    }
}

void load_matrix_from_nvs(void) {
    nvs_handle_t my_handle;
    if (nvs_open("storage", NVS_READONLY, &my_handle) == ESP_OK) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                char key[15];
                snprintf(key, sizeof(key), "cell_%d_%d", r, c);
                int32_t val = 0;
                if (nvs_get_i32(my_handle, key, &val) == ESP_OK) {
                    matrix[r][c] = val;
                }
            }
        }
        nvs_close(my_handle);
    }
}

// --- Servidor HTTP (Serve o HTML) ---
static esp_err_t http_resp_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t matrix_json_handler(httpd_req_t *req) {
    char response[256];
    int offset = snprintf(response, sizeof(response), "{\"rows\":%d,\"cols\":%d,\"values\":[", ROWS, COLS);

    for (int r = 0; r < ROWS && offset > 0 && offset < (int)sizeof(response); r++) {
        offset += snprintf(response + offset, sizeof(response) - offset, "[");
        for (int c = 0; c < COLS && offset > 0 && offset < (int)sizeof(response); c++) {
            offset += snprintf(response + offset, sizeof(response) - offset, "%d%s", matrix[r][c], (c < COLS - 1) ? "," : "");
        }
        offset += snprintf(response + offset, sizeof(response) - offset, "]%s", (r < ROWS - 1) ? "," : "");
    }

    if (offset > 0 && offset < (int)sizeof(response)) {
        snprintf(response + offset, sizeof(response) - offset, "]}");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Matrix response too large");
    return ESP_FAIL;
}

// --- Handler de WebSocket ---
static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        // Envia o estado atual de toda a matriz para o cliente que acabou de ligar
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                char payload[64];
                snprintf(payload, sizeof(payload), "{\"row\":%d,\"col\":%d,\"value\":%d}", r, c, matrix[r][c]);
                httpd_ws_frame_t ws_pkt = {
                    .payload = (uint8_t*)payload,
                    .len = strlen(payload),
                    .type = HTTPD_WS_TYPE_TEXT
                };
                httpd_ws_send_frame_async(req->handle, httpd_req_to_sockfd(req), &ws_pkt);
            }
        }
        return ESP_OK;
    }

    // Leitura dos dados recebidos
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // Obtém o tamanho da mensagem
    httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ws_pkt.len) {
        uint8_t *buf = calloc(1, ws_pkt.len + 1);
        ws_pkt.payload = buf;
        httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);

        // Parse do JSON recebido no formato fixo {"row":x,"col":y,"value":z}
        int r = -1;
        int c = -1;
        int val = 0;
        if (sscanf((char *)ws_pkt.payload, "{\"row\":%d,\"col\":%d,\"value\":%d}", &r, &c, &val) == 3) {
            if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                matrix[r][c] = val;
                save_cell_to_nvs(r, c, val);
                print_matrix();
            }
        }
        free(buf);
    }
    return ESP_OK;
}

// --- Inicialização do Servidor Web ---
void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_get = {
        .uri      = "/",
        .method   = HTTP_GET,
        .handler  = http_resp_handler
    };

    httpd_uri_t matrix = {
        .uri      = "/matrix",
        .method   = HTTP_GET,
        .handler  = matrix_json_handler
    };

    httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .is_websocket = true
    };

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &matrix);
        httpd_register_uri_handler(server, &ws);
    }
}

// --- Inicialização do SoftAP Wi-Fi ---
void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .password = AP_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Access Point iniciado! SSID: %s | Pass: %s", AP_SSID, AP_PASS);
}

void init_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

/* 
// BLOB - Binary Large Object 
// Exemplo de como salvar e carregar uma matriz inteira de uma só vez como um único BLOB na NVS (memória flash) do ESP32.

int32_t map_matrix[ROWS][COLS];

// Guardar a matriz inteira num único BLOB
void save_matrix_to_nvs(void) {
    nvs_handle_t my_handle;
    if (nvs_open("ecu_maps", NVS_READWRITE, &my_handle) == ESP_OK) {
        
        // nvs_set_blob grava o bloco de memória direto da matriz!
        nvs_set_blob(my_handle, "fuel_map_3d", map_matrix, sizeof(map_matrix));
        nvs_commit(my_handle); // Um único commit!
        nvs_close(my_handle);
    }
}

// Ler a matriz inteira de uma vez
void load_matrix_from_nvs(void) {
    nvs_handle_t my_handle;
    if (nvs_open("ecu_maps", NVS_READONLY, &my_handle) == ESP_OK) {
        size_t required_size = sizeof(map_matrix);
        
        // Lê os bytes todos diretamente para a memória da matriz
        nvs_get_blob(my_handle, "fuel_map_3d", map_matrix, &required_size);
        nvs_close(my_handle);
    }
}

*/

/* 
// BLOB - Binary Large Object 
// Exemplo de como salvar e carregar uma STRUCT de uma só vez como um único BLOB na NVS (memória flash) do ESP32.

typedef struct __attribute__((__packed__)) {
    char     wifi_ssid[32];
    char     wifi_pass[64];
    uint16_t rev_limiter;
    float    vbat_calibration;
    uint8_t  flat_shift_enabled;
    uint32_t fuel_map[16][16];
} EcuSettings_t;

EcuSettings_t config;

void save_all_settings(void) {
    nvs_handle_t handle;
    if (nvs_open("config", NVS_READWRITE, &handle) == ESP_OK) {
        // Grava a struct inteira com todas as variáveis num único passo!
        nvs_set_blob(handle, "all_params", &config, sizeof(EcuSettings_t));
        nvs_commit(handle);
        nvs_close(handle);
    }
}

*/


/* 
// Ponto de entrada do programa. 
// ESP32 framework (ESP-IDF) chama app_main() como função principal.

void app_main(void) {
    // 1. Inicializa o armazenamento NVS
    init_nvs();

    // 2. Carrega matriz da Flash e imprime
    load_matrix_from_nvs();
    print_matrix();

    // 3. Inicia Wi-Fi AP e Servidor Web
    wifi_init_softap();
    start_webserver();
}

*/