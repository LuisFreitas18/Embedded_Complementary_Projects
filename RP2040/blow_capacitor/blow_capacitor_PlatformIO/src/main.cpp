#include <Arduino.h>
#include <IRremote.hpp>

// Configuração dos Pinos
const uint8_t IR_RECEIVE_PIN = 17;
const uint8_t RELAY_PIN = 18;
const uint8_t LED_PIN = 1;

// Códigos do teu comando
const uint16_t CMD_LIGAR = 0x45;
const uint16_t CMD_DESLIGAR = 0x19;

// Altera para 'false' se o teu módulo de relé ativar com o pino a HIGH
const bool RELAY_ACTIVE_LOW = true; 

void desligarSistema() {
    if (RELAY_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, HIGH); // Desliga relé Active Low
    } else {
        digitalWrite(RELAY_PIN, LOW);  // Desliga relé Active High
    }
    digitalWrite(LED_PIN, LOW);        // Desliga LED auxiliar
    Serial.println("Sistema: DESLIGADO");
}

void ligarSistema() {
    if (RELAY_ACTIVE_LOW) {
        digitalWrite(RELAY_PIN, LOW);  // Liga relé Active Low
    } else {
        digitalWrite(RELAY_PIN, HIGH); // Liga relé Active High
    }
    digitalWrite(LED_PIN, HIGH);       // Liga LED auxiliar
    Serial.println("Sistema: LIGADO");
}

void setup() {
    Serial.begin(115200);

    // Configuração dos pinos de saída
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Garante que o sistema começa TOTALMENTE DESLIGADO
    desligarSistema();

    // Inicializa o recetor IR
    IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
    Serial.println("Recetor IR pronto. Aguardando comandos...");
}

void loop() {
    if (IrReceiver.decode()) {
        
        // Ignorar comandos repetidos por manter o botão pressionado
        if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
            uint16_t comando = IrReceiver.decodedIRData.command;
            
            Serial.print("Comando recebido: 0x");
            Serial.println(comando, HEX);

            if (comando == CMD_LIGAR) {
                ligarSistema();
            } 
            else if (comando == CMD_DESLIGAR) {
                desligarSistema();
            }
        }
        
        IrReceiver.resume(); // Continua a receber o próximo sinal
    }
}