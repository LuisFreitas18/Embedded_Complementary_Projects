#include <Arduino.h>

#define LED_PIN 4 // Pino físico 3 do ATtiny85 (PB4)

void setup() {
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}