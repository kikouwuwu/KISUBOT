// KISUBOT - Cabeza - Fase 1: hacer parpadear el LED de la placa
//
// Entorno de PlatformIO: cabeza-01-blink
// Placa: ESP32-S3 N16R8 con cámara.
// El LED marcado "IO2" en la placa está conectado al pin 2.

#include <Arduino.h>

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("KISUBOT: la cabeza esta viva");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED encendido");
  delay(500);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED apagado");
  delay(500);
}
