# Plan: la cabeza de KISUBOT

La **cabeza** es la ESP32-S3 N16R8 con cámara. Se encarga del WiFi, la cámara, el audio y la conexión con la IA. Los motores (el **cuerpo**) quedan para después.

Marca cada tarea con `[x]` cuando la termines.

## Avance

| Fase | Estado |
|---|---|
| 0 — Preparar el computador | ✅ Listo |
| 1 — Parpadear un LED | ✅ Funciona |
| 2 — WiFi y control desde el celular | ✅ Funciona |
| 3 — Cámara | ✅ Funciona (se calienta con el video continuo) |
| 4 — Cabeza ↔ cuerpo (UART) | ⏸️ En pausa: faltan pines soldados y cables |
| 5 — IA con la cámara | 🧪 Código listo, falta probarlo en la placa |
| 6 — Audio | ⏳ Esperando micrófono y amplificador |
| 7 — Asistente de voz | ⏳ Después de la fase 6 |

---

## Antes de empezar: pedir ya lo que falta

El envío desde AliExpress demora semanas, así que conviene pedir ahora lo que necesita la cabeza.

- [ ] Micrófono INMP441 — [ver](https://www.aliexpress.us/i/3256809766096721.html). Es digital: los micrófonos de audífonos (como el HyperX Cloud II) son analógicos y no sirven sin un preamplificador.
- [ ] Amplificador MAX98357A — [ver](https://www.aliexpress.us/item/3256810340465738.html)
- [ ] Parlante 40 mm, 3 W, 4 Ω — [ver](https://www.aliexpress.us/item/3256811726937954.html). O sacar uno de un aparato viejo (juguete, parlante de PC o Bluetooth): debe decir 4 u 8 Ω y 1 a 3 W.
- [ ] Cable USB-C que transmita datos (no solo carga)
- [ ] Tira de pines macho (header) para soldar en la ESP32-S3 (fase 4)
- [ ] Cables dupont hembra-hembra (fase 4)

---

## Para avanzar sin la placa

Si estás en un computador donde no puedes conectar nada:

- [ ] Crear la clave de Gemini y probar la pregunta con una foto en AI Studio (ver fase 5).
- [ ] Probar la conexión con Gemini en el simulador Wokwi con [`05_prueba_wokwi`](../firmware/cabeza/05_prueba_wokwi/05_prueba_wokwi.ino).
- [ ] Comprar lo que falta de la lista de arriba.

---

## Fase 0 — Preparar el computador

- [x] Instalar [Arduino IDE 2](https://www.arduino.cc/en/software).
- [x] Agregar las placas ESP32: *Herramientas → Placa → Gestor de placas*, buscar **esp32 by Espressif Systems** e instalar.
- [x] Conectar la ESP32-S3 por el USB-C marcado **USB-UART** por detrás (el otro es USB-OTG).
- [x] Si no aparece el puerto, instalar el driver del chip **CH343**.
- [x] Configurar la placa en *Herramientas*:

| Opción | Valor |
|---|---|
| Placa | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| Puerto | El COM / ttyUSB que aparece al conectar la placa |

**Lo que aprendimos:**
- Arduino recuerda la última configuración, pero revísala antes de subir si cambiaste de placa (la NodeMCU usa **ESP32 Dev Module**).
- Si la subida falla: mantén apretado **BOOT**, aprieta y suelta **EN/RST**, suelta **BOOT** y vuelve a subir.
- Para ver los mensajes desde el principio, abre el Monitor Serie (115200) y aprieta **EN/RST**.
- Cada programa es una carpeta con un `.ino` del mismo nombre. Los archivos extra de la carpeta aparecen como pestañas.

---

## Fase 1 — Primer programa: parpadear un LED

Programa: [`firmware/cabeza/01_blink`](../firmware/cabeza/01_blink/01_blink.ino)

- [x] Abrir el programa, subirlo y ver parpadear el LED marcado **IO2**.
- [x] Abrir el *Monitor Serie* a 115200 baudios y ver los mensajes.

✅ Logro: sabes subir programas a la placa.

---

## Fase 2 — WiFi y control desde el celular

Programa: [`firmware/cabeza/02_wifi_web`](../firmware/cabeza/02_wifi_web/02_wifi_web.ino)

- [x] Copiar `secrets.h.example` como `secrets.h` y poner el nombre y la clave de tu WiFi. Ojo: la ESP32 solo usa redes de **2,4 GHz**.
- [x] Subir el programa y anotar la IP que aparece en el Monitor Serie.
- [x] Abrir esa IP en el navegador del celular (conectado al mismo WiFi) y encender/apagar el LED.

✅ Logro: el robot recibe órdenes desde el celular.

> `secrets.h` está en `.gitignore`: tus claves nunca se suben al repo.

**Lo que aprendimos:** si aparece `fatal error: secrets.h: No such file or directory`, falta el archivo en la carpeta del programa. Hay dos soluciones:
- Crear la pestaña: menú **⋯** (a la derecha de las pestañas) → *Nueva pestaña* → nombre exacto `secrets.h`, y pegar ahí los `#define`.
- O borrar la línea `#include "secrets.h"` y poner los `#define` al inicio del `.ino`. Funciona igual, pero ese `.ino` ya **no se puede subir a GitHub** porque tiene tu clave.

---

## Fase 3 — Cámara

Se usa el ejemplo oficial que trae Arduino.

- [x] Abrir *Archivo → Ejemplos → ESP32 → Camera → CameraWebServer*.
- [x] Elegir el modelo de cámara: descomentar `#define CAMERA_MODEL_ESP32S3_EYE` y comentar los demás (según la versión, está en el `.ino` o en `board_config.h`). Esta placa usa los mismos pines que ese modelo.
- [x] Poner el nombre y la clave del WiFi, subir el programa y abrir la IP en el navegador.
- [x] Probar *Start Stream* y tomar fotos.

✅ Logro: el robot ve.

**Lo que aprendimos:** el video continuo es lento y calienta mucho la cámara, porque la cámara y el WiFi trabajan sin parar. No dejes el stream encendido; si lo usas, baja la resolución a QVGA o VGA. Para el robot basta con sacar fotos cuando hacen falta (eso hace la fase 5).

---

## Fase 4 — Conectar la cabeza con el cuerpo (UART)

⏸️ **En pausa:** la ESP32-S3 viene sin pines soldados. Antes hay que soldarle una tira de pines (header) y conseguir cables dupont hembra-hembra. El cuerpo por ahora es la ESP32 NodeMCU.

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

Usamos **Gemini** porque tiene un nivel gratuito.

Programas: [`firmware/cabeza/05_vision`](../firmware/cabeza/05_vision/05_vision.ino) (en la placa) y [`firmware/cabeza/05_prueba_wokwi`](../firmware/cabeza/05_prueba_wokwi/05_prueba_wokwi.ino) (en el simulador, sin placa)

- [ ] Crear la clave en [aistudio.google.com/apikey](https://aistudio.google.com/apikey) → *Create API key*. Si pide restringirla, marcar solo **Generative Language API**.
- [ ] Probar en AI Studio: subir una foto y preguntar lo mismo que preguntará el robot.
- [ ] (Sin placa) Probar la clave en Wokwi con `05_prueba_wokwi`: debe responder que ve una pelota roja.
- [ ] Instalar la librería **ArduinoJson** (de Benoit Blanchon) en Arduino: *Sketch → Include Library → Manage Libraries*.
- [ ] Crear la carpeta `05_vision` con `05_vision.ino` y un `secrets.h` (copiado de `secrets.h.example`) con el WiFi y la clave.
- [ ] Subir el programa, abrir la IP en el celular y apretar **¿Qué ves?**
- [ ] Anotar qué cámara dice el Monitor Serie (`Sensor: ...`) y si se sigue calentando.

✅ Logro: "¿qué ves?" funciona.

> Nunca subas la API key al repo: va en `secrets.h`.

Ten en cuenta:
- La cámara se enciende solo para sacar la foto y se apaga después, para que no se caliente. Si la segunda foto falla, cambia `APAGAR_CAMARA_ENTRE_FOTOS` a `false`.
- En el nivel gratuito Google usa lo que envías para mejorar sus productos. No apuntes la cámara a documentos ni a personas que no quieran salir.
- Hay un límite de consultas por día. Si aparece **Error 429**, se acabó la cuota: espera o prueba al día siguiente.
- La conexión no verifica el certificado de Google (`setInsecure`). Sirve para probar en casa, pero hay que arreglarlo antes de usar el robot en otras redes.

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
