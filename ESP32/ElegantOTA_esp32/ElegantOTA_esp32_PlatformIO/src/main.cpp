#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

const char* ssid = "MEO-FA2FA0";
const char* password = "8a2dad932b";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // Ligação à rede Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nA ligar ao Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Ligado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Rota simples de teste
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "ESP32 a funcionar com ElegantOTA!");
  });

  // Inicializa o ElegantOTA
  ElegantOTA.begin(&server);
  
  // (Opcional) Define credenciais de acesso à página de update
  // ElegantOTA.setAuth("admin", "admin123");

  server.begin();
  Serial.println("Servidor HTTP e ElegantOTA iniciados.");
}

void loop() {
  // Com AsyncWebServer e ElegantOTA v3, o loop fica limpo!
  ElegantOTA.loop();
}