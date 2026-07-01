#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

// Configurações de WiFi
const char* ssid = "MEO-FA2FA0";
const char* password = "8a2dad932b";

WebServer server(80);

void responderPaginaWeb() {
    // Abre o ficheiro em modo de leitura
    File file = LittleFS.open("/index.html", "r");
    
    if (!file) {
        server.send(500, "text/plain", "Erro Interno: Ficheiro index.html nao encontrado no LittleFS.");
        return;
    }

    // Envia o ficheiro HTML diretamente para o cliente com código HTTP 200 (OK)
    server.streamFile(file, "text/html");
    file.close();
}

// Configuração do Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinos Novos para I2C (para não chocar com o CAN nos pinos 21/22)
#define I2C_SDA 18
#define I2C_SCL 19

String URL = "";

void setup() {
    // Inicializa I2C nos novos pinos
    Wire.begin(I2C_SDA, I2C_SCL);

    // Inicializa Display
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        // Endereço 0x3C é o padrão desses displays
        for(;;); 
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("HTML");
    display.setCursor(0,20);
    display.print("STATUS: OFFLINE");
    display.display();

    Serial.begin(115200);
    delay(1000);

    Serial.println("\nA inicializar o LittleFS...");
    if (!LittleFS.begin(true)) { // O 'true' força a formatação se houver um erro grave
        Serial.println("Erro crítico ao inicializar o LittleFS!");
        return;
    }
    Serial.println("LittleFS montado com sucesso.");

    Serial.print("A ligar a rede: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWi-Fi Conectado!");
    Serial.print("Endereço IP do teu ESP32: http://");
    Serial.println(WiFi.localIP());
    URL = "http://" + WiFi.localIP().toString();

    server.on("/", responderPaginaWeb);
    server.begin();
    Serial.println("Servidor HTTP iniciado!");
}

void updateDisplay(bool status) {
    display.clearDisplay();
    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,20);
    display.print("STATUS: "); 
    display.println(status ? "ONLINE" : "ERRO");
    display.drawLine(0, 12, 128, 12, SSD1306_WHITE);

    display.setCursor(0, 45);
    display.print("URL: ");
    display.println(URL);
    
    display.display();
}

void loop() {
    server.handleClient();
    delay(2); // Pequena pausa para estabilidade do RTOS do ESP32

    // Atualiza o display com o status do servidor
    bool serverOnline = WiFi.status() == WL_CONNECTED;
    updateDisplay(serverOnline);
}