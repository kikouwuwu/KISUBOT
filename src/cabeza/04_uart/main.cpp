// KISUBOT - Cabeza - Fase 4: dar órdenes al cuerpo por UART
//
// Entorno de PlatformIO: cabeza-04-uart (el cuerpo usa cuerpo-04-uart)
// Cada 2 segundos manda "LED_ON" o "LED_OFF" al cuerpo (ESP32 NodeMCU)
// y muestra en el monitor serie lo que el cuerpo responde.
//
// Conexiones (3 cables dupont hembra-hembra):
//   Cabeza GPIO 1  (TX)  ->  Cuerpo GPIO 16 (RX2)
//   Cabeza GPIO 14 (RX)  <-  Cuerpo GPIO 17 (TX2)
//   Cabeza GND           --  Cuerpo GND

#include <Arduino.h>

const int UART_TX = 1;
const int UART_RX = 14;

bool encender = true;
unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  Serial.println("Cabeza lista");
}

void loop() {
  if (millis() - ultimoEnvio >= 2000) {
    ultimoEnvio = millis();
    const char* orden = encender ? "LED_ON" : "LED_OFF";
    Serial1.println(orden);
    Serial.print("Cabeza -> cuerpo: ");
    Serial.println(orden);
    encender = !encender;
  }

  if (Serial1.available()) {
    String respuesta = Serial1.readStringUntil('\n');
    respuesta.trim();
    if (respuesta.length() > 0) {
      Serial.print("Cuerpo -> cabeza: ");
      Serial.println(respuesta);
    }
  }
}
