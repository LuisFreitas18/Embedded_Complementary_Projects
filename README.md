# 🛠️ Embedded Complementary Projects

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Orange?style=flat-square&logo=platformio&logoColor=white)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-blue?style=flat-square&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)

📦 Repositório pessoal com pequenos projectos de *sistemas embarcados (*embedded*)*, hardware e eletrónica aplicada.

Serve como arquivo de aprendizagem, prototipagem e também como base para voltar a usar estes circuitos e sketches mais tarde, sem ter de começar do zero.

---

## 🎛️ Plataformas


| Categoria              | Tecnologias Aplicadas                                    |
| :----------------------- | :--------------------------------------------------------- |
| **Microcontroladores** | Raspberry Pi Pico (RP2040), Espressif ESP32 (WROOM-32)   |
| **Frameworks & IDEs**  | Arduino Framework, PlatformIO, VS Code                   |
| **CAD Eletrónica**    | KiCad (Esquemáticos e Simulação)                      |
| **Linguagens**         | C++ (Firmware), Python (Scripts de Automação / Deploy) |

---

## 📂 Projetos

O repositório está organizado no formato de **Monorepo**, com divisão dos projetos por arquitetura de hardware:

### 🌐 ESP32

* **[httphost_esp32](./ESP32/httphost_esp32/httphost_esp32_PlatformIO):** Servidor HTTP minimalista nativo para controlo remoto de IOs e monitorização de estado via Wi-Fi.

### 📌 RP2040 (Pi Pico)

* **[pwm_led_dc-motor_1mosfet](./RP2040/pwm_led_dc-motor_1mosfet/pwm_led_dc-motor_1mosfet_PlatformIO):** Controlo de velocidade de motor DC e brilho de LEDs por PWM, com utilização de encoders rotativos e isolamento com optoacopladores de alta velocidade (6N137).
* **[i2110_h-bridge_4mosfets_w-leds](./RP2040/i2110_h-bridge_4mosfets_w-leds/i2110_h-bridge_4mosfets_w-leds_PlatformIO):** Implementação de uma ponte H discreta com 4 MOSFETs de canal N, com utilização de drivers de Gate IR2110 para controlo de inversão de marcha e travagem de motores.
* **[blow_capacitor](./RP2040/blow_capacitor/blow_capacitor_PlatformIO):** O objetivo é literalmente explodir um condensador eletrolítico de 22µF / 100V injetando-lhe 220VAC da rede elétrica através de um relé, tudo acionado em segurança via comando de Infravermelhos (IR).

---

## ⚙️ Miscellaneous

### 🚀 Upload Automático para o Pi Pico via Python

Por padrão, o PlatformIO tenta enviar o código para o Pi Pico usando uma porta COM (Serial). Mas quando metes o Pico em modo **BOOTSEL**, ele vira uma pen USB (`RPI-RP2`), o que faz o upload normal falhar.

Para resolver isso, criei um script em Python que faz tudo sozinho:

1. Altera o comportamento do PlatformIO para o modo de upload customizado.
2. Sempre que clicas no botão **Upload**, o script Python entra em ação, deteta automaticamente a drive do Pico (no Windows, Linux ou Mac) e injeta lá o ficheiro `.uf2` compilado.
3. Dá-te feedback com cores vivas no terminal para saberes se correu bem.

Basicamente, passas a ter um botão de Upload funcional via USB Mass Storage sem teres de arrastar ficheiros à mão.

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
