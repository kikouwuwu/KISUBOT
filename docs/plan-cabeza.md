# Plan: la cabeza de KISUBOT

La **cabeza** es la ESP32-S3 N16R8 con cámara. Se encarga del WiFi, la cámara, el audio y la conexión con la IA. Los motores (el **cuerpo**) quedan para después.

Marca cada tarea con `[x]` cuando la termines.

---

## Antes de empezar: pedir ya lo que falta

El envío desde AliExpress demora semanas, así que conviene pedir ahora lo que necesita la cabeza. Las fases 0 a 5 se pueden hacer sin esto.

- [ ] Micrófono INMP441 — [ver](https://www.aliexpress.us/i/3256809766096721.html)
- [ ] Amplificador MAX98357A — [ver](https://www.aliexpress.us/item/3256810340465738.html)
- [ ] Parlante 40 mm, 3 W, 4 Ω — [ver](https://www.aliexpress.us/item/3256811726937954.html)
- [ ] Cable USB-C que transmita datos (no solo carga)

---

## Fase 0 — Preparar el computador

- [ ] Instalar [Arduino IDE 2](https://www.arduino.cc/en/software).
- [ ] Agregar las placas ESP32: *Herramientas → Placa → Gestor de placas*, buscar **esp32 by Espressif Systems** e instalar.
- [ ] Conectar la ESP32-S3 por el USB-C marcado **USB-UART** por detrás (el otro es USB-OTG).
- [ ] Si no aparece el puerto, instalar el driver del chip **CH343**.
- [ ] Configurar la placa en *Herramientas*:

| Opción | Valor |
|---|---|
| Placa | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| Puerto | El COM / ttyUSB que aparece al conectar la placa |

---

## Fase 1 — Primer programa: parpadear un LED

Programa: [`firmware/cabeza/01_blink`](../firmware/cabeza/01_blink/01_blink.ino)

- [ ] Abrir el programa, subirlo y ver parpadear el LED marcado **IO2**.
- [ ] Abrir el *Monitor Serie* a 115200 baudios y ver los mensajes.

✅ Logro: sabes subir programas a la placa.

---

## Fase 2 — WiFi y control desde el celular

Programa: [`firmware/cabeza/02_wifi_web`](../firmware/cabeza/02_wifi_web/02_wifi_web.ino)

- [ ] Copiar `secrets.h.example` como `secrets.h` y poner el nombre y la clave de tu WiFi. Ojo: la ESP32 solo usa redes de **2,4 GHz**.
- [ ] Subir el programa y anotar la IP que aparece en el Monitor Serie.
- [ ] Abrir esa IP en el navegador del celular (conectado al mismo WiFi) y encender/apagar el LED.

✅ Logro: el robot recibe órdenes desde el celular.

> `secrets.h` está en `.gitignore`: tus claves nunca se suben al repo.

---

## Fase 3 — Cámara

Se usa el ejemplo oficial que trae Arduino.

- [ ] Abrir *Archivo → Ejemplos → ESP32 → Camera → CameraWebServer*.
- [ ] Elegir el modelo de cámara: descomentar `#define CAMERA_MODEL_ESP32S3_EYE` y comentar los demás (según la versión, está en el `.ino` o en `board_config.h`). Esta placa usa los mismos pines que ese modelo.
- [ ] Poner el nombre y la clave del WiFi, subir el programa y abrir la IP en el navegador.
- [ ] Probar *Start Stream* y tomar fotos.

✅ Logro: el robot ve.

---

## Fase 4 — Conectar la cabeza con el cuerpo (UART)

Se puede hacer ahora, con la ESP32 NodeMCU como cuerpo.

Programas: [`firmware/cabeza/03_uart_cabeza`](../firmware/cabeza/03_uart_cabeza/03_uart_cabeza.ino) y [`firmware/cuerpo/03_uart_cuerpo`](../firmware/cuerpo/03_uart_cuerpo/03_uart_cuerpo.ino)

- [ ] Conectar los cables:

| Cabeza (ESP32-S3) | Cuerpo (ESP32 NodeMCU) |
|---|---|
| GPIO 1 (TX) | GPIO 16 (RX2) |
| GPIO 14 (RX) | GPIO 17 (TX2) |
| GND | GND |

- [ ] Subir `03_uart_cuerpo` a la NodeMCU (en *Placa* elegir **ESP32 Dev Module**).
- [ ] Subir `03_uart_cabeza` a la ESP32-S3.
- [ ] Ver cómo el LED del cuerpo se enciende y apaga con las órdenes de la cabeza, y las respuestas en el Monitor Serie de la cabeza.

✅ Logro: la cabeza le da órdenes al cuerpo. Después esas órdenes serán "avanza" o "gira".

---

## Fase 5 — IA en la nube con la cámara

- [ ] Crear una cuenta en un servicio de IA con visión (por ejemplo Claude, OpenAI o Gemini) y obtener una **API key**. Es pago por uso.
- [ ] Probar primero desde el computador: mandar una foto y recibir una descripción.
- [ ] Hacer que la ESP32-S3 tome una foto, la envíe y muestre la respuesta en el Monitor Serie o en la página web.

> Nunca subas la API key al repo: va en `secrets.h`.

✅ Logro: "¿qué ves?" funciona.

---

## Fase 6 — Audio (cuando llegue lo pedido)

- [ ] Soldar los pines del micrófono y del amplificador.
- [ ] Conectar según el plan de pines de abajo.
- [ ] Probar el parlante con un tono simple.
- [ ] Probar el micrófono: grabar unos segundos y reproducirlos.

✅ Logro: el robot escucha y habla.

---

## Fase 7 — Asistente de voz

- [ ] Instalar ESP-IDF (lo necesita XiaoZhi).
- [ ] Compilar [XiaoZhi](https://github.com/78/xiaozhi-esp32) con los pines de esta placa.
- [ ] Probar una conversación y revisar qué tan bien funciona en español.
- [ ] Conectar la IA con el cuerpo: que pueda mandar órdenes por UART.

✅ Logro: conversas con el robot y te obedece.

---

## Plan de pines de la cabeza

La cámara, la memoria PSRAM y la ranura SD ocupan muchos pines. Quedan libres el 1, 2, 14, 21, 41, 42 y 47.

| Pin | Uso | Conecta con |
|---|---|---|
| 2 | LED de la placa (IO2) | — |
| 1 | UART TX hacia el cuerpo | GPIO 16 de la NodeMCU |
| 14 | UART RX desde el cuerpo | GPIO 17 de la NodeMCU |
| 41 | I2S BCLK (reloj de audio, compartido) | SCK del INMP441 y BCLK del MAX98357A |
| 42 | I2S WS (compartido) | WS del INMP441 y LRC del MAX98357A |
| 21 | I2S datos del micrófono | SD del INMP441 |
| 47 | I2S datos al amplificador | DIN del MAX98357A |

Alimentación:
- INMP441: 3,3 V y GND; su pin **L/R** a GND.
- MAX98357A: 5 V y GND.

> Es una propuesta: puede cambiar si XiaoZhi necesita el micrófono y el amplificador en pines separados. En ese caso se pueden usar los pines 38, 39 y 40, dejando de usar la ranura SD.
