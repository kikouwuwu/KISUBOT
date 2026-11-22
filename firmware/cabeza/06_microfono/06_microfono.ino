// KISUBOT - Fase 6: prueba del micrófono INMP441
// Muestra qué tan fuerte es el sonido que escucha el micrófono.
// Ábrelo en Herramientas > Serial Plotter (115200) y aplaude o habla cerca.
//
// Conexión (plan de pines de la cabeza):
//   INMP441 VDD -> 3V3   (NUNCA a 5V)
//   INMP441 GND -> GND
//   INMP441 L/R -> GND   (elige el canal izquierdo)
//   INMP441 SCK -> GPIO 41
//   INMP441 WS  -> GPIO 42
//   INMP441 SD  -> GPIO 21
//
// Placa: ESP32S3 Dev Module, con la misma configuración de siempre.
// Funciona aunque la cámara esté conectada: no comparten pines.

#include <ESP_I2S.h>

const int PIN_BCLK = 41;       // SCK del micrófono
const int PIN_WS = 42;         // WS del micrófono
const int PIN_DATOS_MIC = 21;  // SD del micrófono
const int MUESTRAS = 512;

I2SClass i2s;
int32_t muestras[MUESTRAS];
unsigned long ultimoAviso = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nKISUBOT - prueba del microfono");

  i2s.setPins(PIN_BCLK, PIN_WS, -1, PIN_DATOS_MIC);
  if (!i2s.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO, I2S_STD_SLOT_LEFT)) {
    Serial.println("No se pudo iniciar el audio (I2S). Revisa que la placa sea ESP32S3 Dev Module.");
    while (true) delay(1000);
  }
  Serial.println("Listo. Abre Herramientas > Serial Plotter y aplaude cerca del microfono.");
}

void loop() {
  size_t leidas = i2s.readBytes((char*)muestras, sizeof(muestras)) / sizeof(int32_t);
  if (leidas == 0) {
    avisar("No llegan datos del microfono. Revisa los cables SCK (41) y WS (42).");
    return;
  }

  // El INMP441 manda 24 bits dentro de cada número de 32: los corremos 8 lugares
  int64_t suma = 0;
  bool todasIguales = true;
  for (size_t i = 0; i < leidas; i++) {
    muestras[i] >>= 8;
    suma += muestras[i];
    if (muestras[i] != muestras[0]) todasIguales = false;
  }

  if (todasIguales) {
    avisar("El microfono manda siempre lo mismo. Revisa SD (21), L/R a GND y VDD a 3V3.");
    return;
  }

  // Nivel = qué tanto se aleja el sonido del silencio (raíz del promedio de los cuadrados)
  int32_t promedio = suma / (int64_t)leidas;
  double energia = 0;
  for (size_t i = 0; i < leidas; i++) {
    double v = muestras[i] - promedio;
    energia += v * v;
  }
  long nivel = (long)sqrt(energia / leidas);

  // Formato que entiende el Serial Plotter: nombre:valor
  Serial.printf("nivel:%ld\n", nivel);
}

void avisar(const char* mensaje) {
  if (millis() - ultimoAviso > 3000) {
    Serial.println(mensaje);
    ultimoAviso = millis();
  }
  delay(50);
}
