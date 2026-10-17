#ifndef OTA_H
#define OTA_H

#include "esp_http_server.h"
#include "nvs_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o Wi-Fi em modo Access Point (SoftAP).
 * 
 * @param ssid Nome da rede Wi-Fi a criar
 * @param password Palavra-passe da rede (vazio para rede aberta)
 * @param max_conn Número máximo de clientes ligados
 */
void wifi_init_softap(const char *ssid, const char *password, int max_conn);

/**
 * @brief Inicializa o Wi-Fi em modo Station.
 * 
 * @param ssid Nome da rede Wi-Fi a criar
 * @param password Palavra-passe da rede (vazio para rede aberta)
 */
void wifi_init_sta(const char *ssid, const char *password);

/**
 * @brief Arranca o servidor Web HTTP para receber updates OTA via formulário Web.
 * 
 * @return httpd_handle_t Handle do servidor HTTP criado
 */
httpd_handle_t start_webserver(void);

/**
 * @brief Obtém o endereço IP atual (seja em modo Station ou SoftAP).
 * 
 * @param ip_str Buffer onde o IP será escrito (mínimo 16 bytes, ex: char ip[16])
 * @return esp_err_t ESP_OK se o IP for obtido com sucesso
 */
esp_err_t ota_get_ip(char *ip_str);

#ifdef __cplusplus
}
#endif

#endif // OTA_H