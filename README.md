# 🛠️ Embedded Complementary Projects

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Orange?style=flat-square&logo=platformio&logoColor=white)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-ESP--IDF-red?style=flat-square&logo=espressif&logoColor=white)](https://docs.espressif.com/projects/esp-idf/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-blue?style=flat-square&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C-A8B9CC?style=flat-square&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Language](https://img.shields.io/badge/Language-Python-3776AB?style=flat-square&logo=python&logoColor=white)](https://www.python.org/)

📦 Repositório pessoal com pequenos projetos de *sistemas embarcados (embedded)*, hardware e eletrónica aplicada.

Serve como arquivo de aprendizagem, prototipagem e também como base para voltar a usar estes circuitos e sketches mais tarde, sem ter de começar do zero.

---

## 🎛️ Plataformas

| Categoria                    | Tecnologias Aplicadas                                                                |
| :--------------------------- | :----------------------------------------------------------------------------------- |
| **Microcontroladores** | Raspberry Pi Pico (RP2040), Espressif ESP32 (WROOM-32), ATTINY (85)                  |
| **Frameworks & IDEs**  | Arduino Framework, ESP-IDF Framework, PlatformIO, VS Code                            |
| **CAD Eletrónica**    | KiCad (Esquemáticos e Simulação)                                                  |
| **Linguagens**         | C/C++ (Firmware), Python (Scripts de Automação / Deploy / Pre&Post platformio.ini) |

---

## 📂 Projetos

O repositório está organizado no formato de **Monorepo**, com divisão dos projetos por arquitetura de hardware:

### 🌐 ESP32

* **[httphost_esp32](./ESP32/httphost_esp32):** Servidor HTTP minimalista nativo para controlo remoto de IOs e monitorização de estado via Wi-Fi.
* **[ws_ap_matrix_esp32](./ESP32/ws_ap_matrix_esp32):** Aplicação Web embarcada para ESP32 em ESP-IDF que permite controlar e sincronizar bidirecionalmente uma matriz 2D em tempo real via WebSockets. Mantem os dados persistentes na memória NVS.

### 📌 RP2040 (Pi Pico)

* **[pwm_led_dc-motor_1mosfet](./RP2040/pwm_led_dc-motor_1mosfet):** Controlo de velocidade de motor DC e brilho de LEDs por PWM, com utilização de encoders rotativos e isolamento com optoacopladores de alta velocidade (6N137).
* **[i2110_h-bridge_4mosfets_w-leds](./RP2040/i2110_h-bridge_4mosfets_w-leds):** Implementação de uma ponte H discreta com 4 MOSFETs de canal N, com utilização de drivers de Gate IR2110 para controlo de inversão de marcha e travagem de motores.
* **[blow_capacitor](./RP2040/blow_capacitor/blow_capacitor):** O objetivo é literalmente explodir um condensador eletrolítico de 22µF / 100V injetando-lhe 220VAC da rede elétrica através de um relé, tudo acionado em segurança via comando de Infravermelhos (IR).

### 𓇲 ATTINY (85)

* **[ArduinoISP_program_attitny85_using_esp32](./ATTINY/ArduinoISP_program_attitny85_using_esp32):** 2 módulos PlatformIO: Utilizar o ESP32 como programador ArduinoISP para fazer upload de código para ATTINY, neste caso Attiny85; Código de teste simples LED Blink para testar o funcionamento do ATTINY programado com ESP32 ArduinoISP. `platformio.ini` oferece configurações para o clock do Attiny85.

### ⤮  Platform Crossover

* **[ws_ap_matrix_esp32+rp2040_HALimplementation_env](./MCU_PlatformCrossover/ws_ap_matrix_esp32+rp2040_HALimplementation_env):** Protótipo cross-platform (ESP32 / Pi Pico W) com camada Hardware Abstraction Layer (HAL), controlo de matriz 2D via WebSockets, exemplos de armazenamento em BLOB matriz e struct (NVS) e scripts Python para load de credenciais (.env).

### 🪫 Sem MCU, apenas eletrónica e circuitos

* **[boost_converter_UC3843_5Vto12V](./NO_MCU(ElectronicsOnly)/boost_converter_UC3843_5Vto12V):** Conversor Boost (Step-Up) de 5V para 12V, baseado no controlador PWM UC3843. Implementação de um conversor DC-DC THT, capaz de elevar uma tensão de entrada de 5V (por exemplo, de uma porta USB2.0) para uma saída regulada de 12V. Inclui esquemáticos, layout de PCB e render 3D (KiCad).

---

## ⚙️ Miscellaneous

### 🚀 Upload Automático para o Pi Pico via Python

Por padrão, o PlatformIO tenta enviar o código para o Pi Pico usando uma porta COM (Serial). Mas quando metes o Pico em modo **BOOTSEL**, ele vira uma pen USB (`RPI-RP2`), o que faz o upload normal falhar.

Para resolver isso, criei um script em Python que faz tudo sozinho:

1. Altera o comportamento do PlatformIO para o modo de upload customizado.
2. Sempre que clicas no botão **Upload**, o script Python entra em ação, deteta automaticamente a drive do Pico (no Windows, Linux ou Mac) e injeta lá o ficheiro `.uf2` compilado.
3. Dá-te feedback com cores vivas no terminal para saberes se correu bem.

Basicamente, passas a ter um botão de Upload funcional via USB Mass Storage sem teres de arrastar ficheiros à mão.

### 🔐 Credenciais em (.env) via Python

Para evitar expor o SSID e a Palavra-passe do Access Point (AP) no controlo de versão (Git), a configuração de rede é injetada automaticamente durante o processo de compilação:

1. O script `load_ap_env.py` é executado antes da compilação iniciar (`extra_scripts = pre:load_ap_env.py`) e lê o ficheiro privado `src/.env`.
2. Gera automaticamente o cabeçalho C `src/ap_config.h` com os `#define` contendo o SSID e a palavra-passe lidos.
3. Garante que as credenciais reais ficam protegidas e fora do repositório, disponibilizando apenas o modelo público `.env.example`.

---

## 🚀 Clonar e Configurar

Para evitar conflitos de compilação entre o Plataformas diferentes, este repositório utiliza **Multi-root Workspaces** do VS Code.

1. Faz o clone do repositório para a tua máquina:

   ```bash
   git clone [https://github.com/LuisFreitas18/Embedded_Complementary_Projects.git](https://github.com/LuisFreitas18/Embedded_Complementary_Projects.git)
   ```
2. No VS Code, vai a **File ➔ Open Workspace from File...** e escolhe o ficheiro:
   `Embedded_Complementary_Projects.code-workspace`

Cada projeto vai aparecer como uma pasta separada na barra lateral e PlatformIO vai reconhecer tudo.
