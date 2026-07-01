// Raspberry PI Pico, Platformio, Framwork: Arduino, C++
#include <Arduino.h>

// Rotary Encoder GPIO
const uint8_t RE_CLK = 13;
const uint8_t RE_DT = 14;
const uint8_t RE_SW = 15;

// Mosfet Gate PWM Signal GPIO (IR2110 Half Bridge Gate Driver w/ Bootstrap)
const uint8_t PWM_IR2110_LEFT_LOW_PIN = 18;
const uint8_t PWM_IR2110_LEFT_HIGH_PIN = 19;

const uint8_t PWM_IR2110_RIGHT_LOW_PIN = 16;
const uint8_t PWM_IR2110_RIGHT_HIGH_PIN = 17;

// LED H-Bridge Observation GPIO
const uint8_t LED_HBRIDGE_LEFT_HIGH_PIN = 3;
const uint8_t LED_HBRIDGE_LEFT_LOW_PIN = 4;

const uint8_t LED_HBRIDGE_RIGHT_HIGH_PIN = 6;
const uint8_t LED_HBRIDGE_RIGHT_LOW_PIN = 7;

// Volatile (Interruption) 
volatile int16_t pwmValue = 0; // Começa a meio termo (0 a 255)
volatile bool updatePwm = true;  // Flag para atualizar o PWM no loop

int lastClkState;
bool direction = true; // true = Horário (Frente), false = Anti-horário (Trás)

void turnOffBridgeSafely() {
    // Desligar gerador de PWM
    analogWrite(PWM_IR2110_LEFT_HIGH_PIN, 0);
    analogWrite(PWM_IR2110_RIGHT_HIGH_PIN, 0);
    analogWrite(LED_HBRIDGE_LEFT_HIGH_PIN, 0);
    analogWrite(LED_HBRIDGE_RIGHT_HIGH_PIN, 0);

    // Pinos Low-Side são digitais, desligar diretamente
    digitalWrite(PWM_IR2110_LEFT_LOW_PIN, LOW);
    digitalWrite(PWM_IR2110_RIGHT_LOW_PIN, LOW);
    digitalWrite(LED_HBRIDGE_LEFT_LOW_PIN, LOW);
    digitalWrite(LED_HBRIDGE_RIGHT_LOW_PIN, LOW);

    delayMicroseconds(10); // Deadtime de hardware via software (10 microssegundos)
}

void updateGateDrivers(int16_t pwmValueFunc, bool direction) {
    turnOffBridgeSafely(); // Evita shoot-through entre High e Low side MOSFETs (Mesma Half-Bridge)

    if (pwmValueFunc == 0) return; // Se o PWM for zero, não ativa nada

    if (direction) {
        // Horário
        // Left Low Side ON + Right High Side PWM

        // Liga o pino digital Low Side oposto
        digitalWrite(PWM_IR2110_LEFT_LOW_PIN, HIGH);
        digitalWrite(LED_HBRIDGE_LEFT_LOW_PIN, HIGH);

        // Aplica o PWM no High Side correspondente
        analogWrite(PWM_IR2110_RIGHT_HIGH_PIN, pwmValueFunc);
        analogWrite(LED_HBRIDGE_RIGHT_HIGH_PIN, pwmValueFunc);
    } else {
        // Anti-horário
        // Right Low Side ON + Left High Side PWM

        // Liga o pino digital Low Side oposto
        digitalWrite(PWM_IR2110_RIGHT_LOW_PIN, HIGH);
        digitalWrite(LED_HBRIDGE_RIGHT_LOW_PIN, HIGH);

        // Aplica o PWM no High Side correspondente
        analogWrite(PWM_IR2110_LEFT_HIGH_PIN, pwmValueFunc);
        analogWrite(LED_HBRIDGE_LEFT_HIGH_PIN, pwmValueFunc);
    }
}

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

    pinMode(PWM_IR2110_LEFT_LOW_PIN, OUTPUT);
    pinMode(PWM_IR2110_LEFT_HIGH_PIN, OUTPUT);
    pinMode(PWM_IR2110_RIGHT_LOW_PIN, OUTPUT);
    pinMode(PWM_IR2110_RIGHT_HIGH_PIN, OUTPUT);

    pinMode(LED_HBRIDGE_LEFT_HIGH_PIN, OUTPUT);
    pinMode(LED_HBRIDGE_LEFT_LOW_PIN, OUTPUT);
    pinMode(LED_HBRIDGE_RIGHT_HIGH_PIN, OUTPUT);
    pinMode(LED_HBRIDGE_RIGHT_LOW_PIN, OUTPUT);

    // Garantir que a Ponte H arranca totalmente desligada
    turnOffBridgeSafely();

    lastClkState = digitalRead(RE_CLK);
    attachInterrupt(digitalPinToInterrupt(RE_CLK), readEncoder, CHANGE);
    
    Serial.print("Sistema Iniciado. PWM Inicial: ");
    Serial.println(pwmValue);
}

void loop() {
    if (updatePwm) {
        updatePwm = false; // Reseta a flag

        // Atualiza fisicamente as saídas dos MOSFETs de forma segura
        updateGateDrivers(pwmValue, direction);

        float dutyCycle = (pwmValue / 255.0) * 100.0; // Calcula o ciclo de trabalho em %
        Serial.print("PWM Ajustado para: ");
        Serial.print(pwmValue);
        Serial.print(" | Duty Cycle: ");
        Serial.print(dutyCycle, 1); // 1 casa decimal
        Serial.println("%)");
    }

    // SW do Rotary Encoder para RESET PWM a 0 e INVERSÃO DE POLARIDADE
    if (digitalRead(RE_SW) == LOW) {
        delay(50); // Debounce
        if (digitalRead(RE_SW) == LOW) {
            
            direction = !direction; // Inverte a direção
            updatePwm = true;
            
            Serial.println("INVERSAO DE SENTIDO DETETADA: " + String(direction ? "HORARIO" : "ANTI-HORARIO") + " | PWM em 0%");
            while(digitalRead(RE_SW) == LOW); // Espera largar o botão
            delay(50); 
        }
    }
}