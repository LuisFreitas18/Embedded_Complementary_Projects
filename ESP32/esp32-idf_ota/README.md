
# OTA ESP32

## Estrutura de Ficheiros

* `main.cpp`: Ponto de entrada do programa, inicializa o NVS, gere o Wi-Fi, valida o rollback e arranca o servidor.
* `ota.cpp`: Lógica do servidor HTTP (`/` e `/update`), gravação da flash via OTA e controlo opcional do LED.
* `ota.h`: Declarações das funções de Wi-Fi, servidor web e obtenção de IP.
* `ota_webpage.h`: Código HTML, CSS e JavaScript da página de upload.

## Modos de Wi-Fi (AP ou STA)

O sistema suporta dois modos de funcionamento para o Wi-Fi:

* **Modo STA (Station):** O ESP32 liga-se a um router existente (por exemplo, a tua rede doméstica).
* **Modo AP (Access Point):** O ESP32 cria a sua própria rede Wi-Fi aberta ou protegida, permitindo ligar diretamente a ele (ideal para configurar no terreno).

## Funções Disponíveis em `ota.h`

```
// Inicializa o ESP32 como Access Point (cria rede Wi-Fi própria)
void wifi_init_softap(const char *ssid, const char *password, int max_conn);

// Inicializa o ESP32 em modo Station (liga a um router)
void wifi_init_sta(const char *ssid, const char *password);

// Arranca o servidor HTTP OTA
httpd_handle_t start_webserver(void);

// Obtém o endereço IP atual do ESP32 (guarda na string fornecida)
esp_err_t ota_get_ip(char *ip_str);
```
