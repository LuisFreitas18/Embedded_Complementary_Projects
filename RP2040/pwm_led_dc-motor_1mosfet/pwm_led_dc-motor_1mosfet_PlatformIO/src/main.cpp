#include <Arduino.h>

// Rotary Encoder GPIO
const uint8_t RE_CLK = 0;
const uint8_t RE_DT = 1;
const uint8_t RE_SW = 2;

// Mosfet Gate PWM Signal GPIO
const uint8_t PWM_PIN = 15;

// Volatile (Interruption) 
volatile int16_t pwmValue = 0; // Começa a meio termo (0 a 255)
volatile bool updatePwm = true;  // Flag para atualizar o PWM no loop

int lastClkState;

// No Pi Pico, para correr na RAM, usa-se a macro __not_in_flash_func antes da função
void __not_in_flash_func(readEncoder)() {
    int clkState = digitalRead(RE_CLK);
    int dtState = digitalRead(RE_DT);

    // Se o estado do CLK mudou, houve rotação
    if (clkState != lastClkState && clkState == LOW) {
        // Determina a direção da rotação
        if (dtState == HIGH) {
            if (pwmValue < 255) pwmValue += 5; // Sobe o PWM de 5 em 5
        } else {
            if (pwmValue > 0) pwmValue -= 5;   // Desce o PWM de 5 em 5
        }
        
        // Garante que o valor não sai dos limites do PWM (0-255)
        if (pwmValue > 255) pwmValue = 255;
        if (pwmValue < 0) pwmValue = 0;

        updatePwm = true; // Avisa o loop para atualizar o PWM
    }
    lastClkState = clkState;
}

void setup() {
    Serial.begin(115200);

    pinMode(RE_CLK, INPUT_PULLUP);
    pinMode(RE_DT, INPUT_PULLUP);
    pinMode(RE_SW, INPUT_PULLUP);

    pinMode(PWM_PIN, OUTPUT);

    lastClkState = digitalRead(RE_CLK);

    attachInterrupt(digitalPinToInterrupt(RE_CLK), readEncoder, CHANGE);

    analogWrite(PWM_PIN, pwmValue); // Inicializa o PWM com o valor inicial
    Serial.print("Sistema Iniciado. PWM Inicial: ");
    Serial.println(pwmValue);
}

void loop() {
    if (updatePwm) {
        updatePwm = false; // Reseta a flag

        analogWrite(PWM_PIN, pwmValue); // Atualiza o PWM com o novo valor

        float dutyCycle = (pwmValue / 255.0) * 100.0; // Calcula o ciclo de trabalho em %
        Serial.print("PWM Ajustado para: ");
        Serial.print(pwmValue);
        Serial.print(" | Duty Cycle: ");
        Serial.print(dutyCycle, 1); // 1 casa decimal
        Serial.println("%)");
    }

    // Podes usar o botão do encoder (SW) para fazer mute/reset, se desejares
    if (digitalRead(RE_SW) == LOW) {
        delay(50); // Debounce
        if (digitalRead(RE_SW) == LOW) {
            pwmValue = 0;

            analogWrite(PWM_PIN, pwmValue);

            updatePwm = true;
            Serial.println("RESET | PWM em 0%");
            while(digitalRead(RE_SW) == LOW); // Espera largar o botão
        }
    }
}