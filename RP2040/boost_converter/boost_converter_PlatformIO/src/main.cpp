#include <Arduino.h>

// Definições dos pinos
const int inputPin = 15; // Pino de entrada (GPIO 15)
const int outputPin = 25; // Pino de saída (GPIO 25)
const int mosfetPin = 28; // Pino do MOSFET (GPIO 28)

// Variáveis globais
int inputVoltage = 0;
int outputVoltage = 0;

void setup() {
  // Configuração dos pinos
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
  pinMode(mosfetPin, OUTPUT);

  // Inicializa a saída PWM no pino do MOSFET
  analogWriteFrequency(mosfetPin, 1000); // Frequência da PWM (1 kHz)
  analogWrite(mosfetPin, 0); // Inicia com duty cycle zero

  // Configuração da comunicação serial
  Serial.begin(9600);
}

void loop() {
  // Leitura da tensão de entrada
  inputVoltage = analogRead(inputPin);

  // Cálculo do duty cycle para o MOSFET
  int dutyCycle = map(inputVoltage, 0, 4095, 0, 255); // Mapeia a tensão de entrada (0-4095) para duty cycle (0-255)

  // Controle do MOSFET via PWM
  analogWrite(mosfetPin, dutyCycle);

  // Impressão da tensão de saída estimada
  outputVoltage = map(dutyCycle, 0, 255, 0, 4095); // Mapeia o duty cycle para uma tensão de saída estimada (0-4095)
  Serial.print("Input Voltage: ");
  Serial.print(inputVoltage);
  Serial.print(", Output Voltage: ");
  Serial.println(outputVoltage);

  delay(100); // Intervalo de 100 ms
}