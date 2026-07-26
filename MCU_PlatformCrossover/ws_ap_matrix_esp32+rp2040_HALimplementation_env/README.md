# 🎛️ ESP32 2D Matrix Controller (HAL & Multi-NVS)

Projeto em C com **ESP-IDF** focado na implementação de Ponto de Acesso Wi-Fi (SoftAP), sincronização WebSocket em tempo real e técnicas de persistência em memória Flash (NVS por elemento e via BLOB).

---

## ⚡ Código Essencial (Cheat Sheet ESP-IDF)

### 1. Inicialização da NVS e carregamento da matriz
O arranque do firmware restaura o estado anterior da matriz a partir da flash:

```c
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

extern int matrix[ROWS][COLS];

void init_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void load_matrix_from_nvs(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                char key[15];
                snprintf(key, sizeof(key), "cell_%d_%d", r, c);
                int32_t val = 0;
                if (nvs_get_i32(handle, key, &val) == ESP_OK) {
                    matrix[r][c] = val;
                }
            }
        }
        nvs_close(handle);
    }
}
```

### 2. Persistência de uma célula individual na flash
Cada alteração recebida pelo WebSocket pode ser guardada imediatamente numa chave NVS:

```c
void save_cell_to_nvs(int r, int c, int val) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        char key[15];
        snprintf(key, sizeof(key), "cell_%d_%d", r, c);
        nvs_set_i32(handle, key, val);
        nvs_commit(handle);
        nvs_close(handle);
    }
}
```

### 3. Exemplos de BLOB: matriz e struct
Exemplo de gravação de uma matriz inteira num único blob na NVS:

```c
int32_t matrix_blob[ROWS][COLS];

void save_matrix_blob(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_blob(handle, "matrix_blob", matrix_blob, sizeof(matrix_blob));
        nvs_commit(handle);
        nvs_close(handle);
    }
}

void load_matrix_blob(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        size_t size = sizeof(matrix_blob);
        nvs_get_blob(handle, "matrix_blob", matrix_blob, &size);
        nvs_close(handle);
    }
}
```

Exemplo de gravação de uma struct completa num único blob:

```c
typedef struct {
    char ssid[32];
    char password[64];
    uint8_t brightness;
    uint8_t enabled;
} app_settings_t;

app_settings_t settings;

void save_settings_blob(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READWRITE, &handle) == ESP_OK) {
        nvs_set_blob(handle, "app_settings", &settings, sizeof(settings));
        nvs_commit(handle);
        nvs_close(handle);
    }
}

void load_settings_blob(void) {
    nvs_handle_t handle;
    if (nvs_open("storage", NVS_READONLY, &handle) == ESP_OK) {
        size_t size = sizeof(settings);
        nvs_get_blob(handle, "app_settings", &settings, &size);
        nvs_close(handle);
    }
}
```

### 4. Configuração do Wi-Fi em Modo Access Point (SoftAP)
Estrutura mínima para arrancar o ponto de acesso Wi-Fi local:

```c
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "ap_config.h"

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
}
```

### 5. Atualização da matriz via WebSocket
Quando o cliente envia uma atualização, o firmware valida as coordenadas e grava a nova célula:

```c
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
```

### 6. Ponto de entrada do firmware
O arranque do programa segue este fluxo simples:

```c
void app_main(void) {
    init_nvs();
    load_matrix_from_nvs();
    print_matrix();
    wifi_init_softap();
    start_webserver();
}
```

---

## 🛠️ Destaques da Implementação

- **Gestão Modular (HAL):** Inclusão de `hal_mcu.h`, `config.h` e `ap_config.h` para separação entre hardware e aplicação.
- **Credenciais Protegidas (.env):** Injeção das variáveis `AP_SSID` e `AP_PASS` em tempo de compilação sem expor senhas no repositório.
- **Endpoint REST `/matrix`:** Serviço JSON dedicado que devolve a matriz completa (`{"rows": X, "cols": Y, "values": [...]}`).
- **Comunicação WebSocket (`/ws`):** Troca contínua de dados entre o cliente web e o microcontrolador sem recarregar a página.

---

## 📲 Como usar

1. Compila e faz upload via PlatformIO.
2. Conecta-te à rede Wi-Fi criada pelo ESP32.
3. Acede aos endpoints:
   - **Interface Web:** `http://192.168.4.1/`
   - **Leitura JSON da Matriz:** `http://192.168.4.1/matrix`

---

## 📂 Ficheiros do Projeto

- [ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/main.c](ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/main.c) — NVS, handlers HTTP/WS, endpoint REST e exemplos de gravação BLOB.
- [ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/ap_config.h](ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/ap_config.h) — Header gerado automaticamente pelo script Python com as definições do AP.
- [ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/config.h](ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/config.h) & [ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/hal_mcu.h](ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/hal_mcu.h) — Dimensões da matriz e abstração de hardware.
- [ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/web_page.h](ws_ap_matrix_esp32+rp2040_HALimplementation_env_PlatformIO/src/web_page.h) — Interface Web embutida em raw string C.

---

## ⚠️ Notas

- Se a resposta JSON em `/matrix` exceder 256 bytes, o servidor devolve erro HTTP 500.
- A gravação via BLOB reduz o desgaste da memória Flash comparativamente à gravação chave-valor por elemento individual.
