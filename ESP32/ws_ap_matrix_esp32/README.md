# 🎛️ ESP32 2D Matrix Web Controller (ESP-IDF)

Projeto de bancada em C com **ESP-IDF** para ESP32 Dev Module. Cria um Access Point Wi-Fi, serve uma página HTML e sincroniza uma matriz 2D via WebSockets em tempo real, guardando os dados na memória Flash (NVS).

---

## 🛠️ O que faz

- **Access Point Wi-Fi:** Cria uma rede Wi-Fi própria. Não precisa de router.
- **Servidor Web & WebSockets:** Serve a página e troca dados bidirecionais sem recarregar o navegador.
- **Sincronização:** Alterações na matriz enviam mensagens JSON que atualizam a RAM do ESP32.
- **Memória Flash (NVS):** Grava cada valor alterado na NVS. Mantém os dados ao desligar da corrente.

---

## 📐 Lógica do Firmware

- **HTTP / WS (`esp_http_server`):** Serve a página em `/` e faz o handshake de WebSocket em `/ws`.
- **JSON:** Lida com o formato `{"row": x, "col": y, "value": z}` na receção e envio.
- **NVS:** Usa `nvs_set_i32` e `nvs_get_i32` na partição `storage`.

---

## 📲 Como Usar

1. Compila e faz upload do código via PlatformIO.
2. Liga-te à rede Wi-Fi do ESP32:
   - **SSID:** `ESP32_Matriz_AP`
   - **Password:** `12345678password`
3. Abre o navegador no IP:
   [http://192.168.4.1](http://192.168.4.1)
4. Altera os valores na grelha. Reinicia o ESP32 para confirmar que os dados foram mantidos.

---

## 📂 Ficheiros

- [`src/main.c`](src/main.c) — NVS, Wi-Fi AP, handlers de WebSocket e loop principal.
- [`src/web_page.h`](src/web_page.h) — HTML, CSS e JS numa raw string C.
- [`src/CMakeLists.txt`](src/CMakeLists.txt) — Configuração de build e dependências CMake.
- [`platformio.ini`](platformio.ini) — Configuração do projeto no PlatformIO.

---

## ⚠️ Notas Práticas

- Se o telemóvel avisar "Sem Internet", escolhe **Manter ligado** ou desliga os dados móveis.
- Acompanha as leituras/escritas na NVS pelo Serial Monitor a `115200` baud.

---

## 📂 Versão Avançada (Cross-Platform)

Na pasta [`MCU_PlatformCrossover`](./MCU_PlatformCrossover) encontras a versão melhorada e mais complexa deste projeto. Essa versão inclui:

- Camada de abstração de hardware (**HAL**) com suporte para **ESP32** e **Raspberry Pi Pico W**.
- Script Python para automação de ficheiros `.env` (manter credenciais privadas).
- Armazenamento em NVS com suporte a **BLOBs** para gravar estruturas de dados e matrizes completas.
- Explicações detalhadas e exemplos avançados de configuração de AP no ESP-IDF.