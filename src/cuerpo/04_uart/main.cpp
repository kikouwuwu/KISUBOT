// KISUBOT - Cuerpo - Fase 4: recibir órdenes de la cabeza por UART
//
// Entorno de PlatformIO: cuerpo-04-uart (la cabeza usa cabeza-04-uart)
// Placa: ESP32 NodeMCU (ESP-32S).
// Escucha las órdenes de la cabeza, enciende o apaga el LED del pin 2
// y responde "OK". Si tu placa no tiene LED en el pin 2, conecta uno
// con una resistencia de 220 ohm entre el pin 2 y GND.
//
// Conexiones:
//   Cuerpo GPIO 16 (RX2)  <-  Cabeza GPIO 1  (TX)
//   Cuerpo GPIO 17 (TX2)  ->  Cabeza GPIO 14 (RX)
//   Cuerpo GND            --  Cabeza GND
// En algunas placas el 16 y el 17 aparecen como 16/17, G16/G17 o IO16/IO17.
// Si tu placa no los tiene, cambia UART_RX y UART_TX por otros pines libres
// (por ejemplo 25 y 26) y conecta los cables ahí.

#include <Arduino.h>

const int UART_RX = 16;
const int UART_TX = 17;
const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Cuerpo listo");
}

void loop() {
  if (Serial2.available()) {
    String orden = Serial2.readStringUntil('\n');
    orden.trim();
    if (orden.length() == 0) {
      return;
    }

    Serial.print("Orden recibida: ");
    Serial.println(orden);

    if (orden == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial2.println("OK LED_ON");
    } else if (orden == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial2.println("OK LED_OFF");
    } else {
      Serial2.print("ERROR orden desconocida: ");
      Serial2.println(orden);
    }
  }
}
