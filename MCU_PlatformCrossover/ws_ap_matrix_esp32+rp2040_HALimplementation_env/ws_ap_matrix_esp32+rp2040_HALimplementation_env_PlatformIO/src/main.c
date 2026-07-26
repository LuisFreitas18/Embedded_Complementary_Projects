#if defined(USING_ESP32) // ESP32 framework (ESP-IDF)

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal_mcu.h"

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


#elif defined(USING_RP2040W) // RP2040 framework (Arduino)

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void setup() {
    
}

void loop() {
    
}

#ifdef __cplusplus
}
#endif

#endif