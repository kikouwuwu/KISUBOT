// KISUBOT - Cabeza - Paso 2: WiFi y control desde el celular
//
// La placa se conecta a tu WiFi y crea una página web con dos botones
// para encender y apagar el LED. Ábrela desde el celular con la IP que
// aparece en el Monitor Serie (115200 baudios).
//
// Antes de subir: copia "secrets.h.example" como "secrets.h" y pon
// el nombre y la clave de tu WiFi (solo redes de 2,4 GHz).

#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

const int LED_PIN = 2;

WebServer server(80);
bool ledEncendido = false;

String pagina() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>KISUBOT</title></head>"
                "<body style='font-family:sans-serif;text-align:center;margin-top:40px'>"
                "<h1>KISUBOT</h1>";
  html += "<p>LED: <b>";
  html += ledEncendido ? "encendido" : "apagado";
  html += "</b></p>";
  html += "<p><a href='/on'><button style='font-size:24px;padding:10px 20px'>Encender</button></a></p>";
  html += "<p><a href='/off'><button style='font-size:24px;padding:10px 20px'>Apagar</button></a></p>";
  html += "</body></html>";
  return html;
}

void volverAlInicio() {
  server.sendHeader("Location", "/");
  server.send(303);
}

void manejarInicio() {
  server.send(200, "text/html", pagina());
}

void manejarEncender() {
  ledEncendido = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Orden desde la web: encender LED");
  volverAlInicio();
}

void manejarApagar() {
  ledEncendido = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("Orden desde la web: apagar LED");
  volverAlInicio();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. Abre en el celular: http://");
  Serial.println(WiFi.localIP());

  server.on("/", manejarInicio);
  server.on("/on", manejarEncender);
  server.on("/off", manejarApagar);
  server.begin();
}

void loop() {
  server.handleClient();
}
