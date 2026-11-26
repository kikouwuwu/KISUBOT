# Plan: la cabeza de KISUBOT

La **cabeza** es la ESP32-S3 N16R8 con cámara. Se encarga del WiFi, la cámara, el audio y la conexión con la IA. Los motores (el **cuerpo**) quedan para después.

Marca cada tarea con `[x]` cuando la termines.

## Avance

| Fase | Estado |
|---|---|
| 0 - Preparar el computador | Listo con PlatformIO (fases 1, 2 y 4 probadas en la placa) |
| 1 - Parpadear un LED | Funciona |
| 2 - WiFi y control desde el celular | Funciona |
| 3 - Cámara | Funciona (se calienta con el video continuo) |
| 4 - Cabeza y cuerpo por UART | Funciona |
| 5 - IA con la cámara | Clave y conexión probadas en Wokwi; falta probar con la cámara en la placa |
| 6 - Audio | Prueba del micrófono lista; esperando micrófono y amplificador |
| 7 - Asistente de voz | Pendiente, después de la fase 6 |

---

## Antes de empezar: pedir ya lo que falta

El envío desde AliExpress demora semanas, así que conviene pedir ahora lo que necesita la cabeza.

- [ ] Micrófono INMP441 - [ver](https://www.aliexpress.us/i/3256809766096721.html). Es digital: los micrófonos de audífonos (como el HyperX Cloud II) y el módulo MAX4466 son analógicos y no sirven. En Global Electrónica (Viña) lo venden como ["INMP411"](https://globalelectronica.cl/5044-sonido/inmp411.html) ($4.990 al 20-11-2026); su ficha corresponde al INMP441 (I2S, pines SCK, WS, L/R y SD).
- [ ] Amplificador MAX98357A - [ver](https://www.aliexpress.us/item/3256810340465738.html). No lo tienen en Global Electrónica; alternativa más rápida: Mercado Libre Chile.
- [ ] Parlante de 4 u 8 Ω y 1 a 3 W. En Global Electrónica: mini parlante 40 mm, 2 W, 8 Ω (código 279401, $1.650 al 20-11-2026). O sacar uno de un aparato viejo (juguete, parlante de PC o Bluetooth).
- [ ] Cable USB-C que transmita datos (no solo carga)
- [x] Tira de pines macho (header) para soldar en la ESP32-S3 (fase 4)
- [x] Cables dupont hembra-hembra (fase 4)

---

## Para avanzar sin la placa

Si estás en un computador donde no puedes conectar nada:

- [ ] Crear la clave de Gemini y probar la pregunta con una foto en AI Studio (ver fase 5).
- [ ] Probar la conexión con Gemini en el simulador Wokwi con [`src/cabeza/05_prueba_wokwi`](../src/cabeza/05_prueba_wokwi/main.cpp).
- [ ] Compilar los entornos en PlatformIO (*Build*): no necesita la placa y avisa si algo está mal escrito.
- [ ] Comprar lo que falta de la lista de arriba.

---

## Fase 0 - Preparar el computador

Las fases 0 a 3 se hicieron con Arduino IDE. Desde ahora el proyecto usa **PlatformIO**.

- [ ] Instalar [VS Code](https://code.visualstudio.com/) y la extensión **PlatformIO IDE** (al abrir el repo, VS Code también la sugiere).
- [ ] Abrir la carpeta del repo en VS Code (*File → Open Folder*). PlatformIO reconoce el `platformio.ini` de la raíz y descarga solo la plataforma y las librerías la primera vez (tarda unos minutos).
- [ ] Copiar `include/secrets.h.example` como `include/secrets.h` y poner tus datos (WiFi y clave de Gemini).
- [ ] Elegir el entorno `cabeza-01-blink` en la barra inferior de PlatformIO y apretar *Build*: si termina en `SUCCESS`, el computador está listo.
- [ ] Si el puerto de una placa no aparece, instalar el driver: **CP2102** para la NodeMCU y **CH343** para el puerto UART de la ESP32-S3. El USB nativo de la ESP32-S3 no necesita driver en Windows 10 u 11.

### Cómo se usa PlatformIO

Todo el firmware es **un solo proyecto**. Cada programa es un **entorno**, y la placa, la memoria y las librerías ya están configuradas en `platformio.ini`.

| Entorno | Placa | Programa |
|---|---|---|
| `cabeza-01-blink` | ESP32-S3 | Parpadear el LED |
| `cabeza-02-wifi-web` | ESP32-S3 | Página web para el LED |
| `cabeza-04-uart` | ESP32-S3 | Órdenes al cuerpo por UART |
| `cabeza-05-vision` | ESP32-S3 | "¿Qué ves?" con la cámara y Gemini |
| `cabeza-05-prueba-wokwi` | Simulador Wokwi | Prueba de Gemini sin placa |
| `cabeza-06-microfono` | ESP32-S3 | Nivel de sonido del micrófono |
| `cuerpo-04-uart` | ESP32 NodeMCU | Recibir órdenes de la cabeza |

1. Elegir el entorno en la barra inferior de VS Code (*Switch PlatformIO Project Environment*).
2. *Build* compila, *Upload* sube el programa a la placa y *Serial Monitor* muestra los mensajes.
3. Desde la terminal: `pio run -e cabeza-01-blink -t upload` y `pio device monitor -e cabeza-01-blink`.

**Puertos USB:**
- **Cabeza:** con la configuración actual (`ARDUINO_USB_CDC_ON_BOOT=1`), los mensajes salen por el **USB-C nativo** (marcado USB u OTG), y ese mismo puerto sirve para subir. Si prefieres el puerto UART (chip CH343), cambia esa línea a `0` en la sección `[cabeza]` de `platformio.ini`.
- **Cuerpo:** la NodeMCU tiene un solo USB.
- PlatformIO busca el puerto solo. Si con las dos placas conectadas elige la equivocada, copia `platformio_local.ini.example` como `platformio_local.ini` y pon ahí el COM de cada una (ese archivo no se sube al repo).

**Lo que aprendimos:**
- Si la subida se queda en "Connecting...": mantén apretado **BOOT**, aprieta y suelta **EN/RST**, suelta **BOOT** y vuelve a subir.
- Para ver los mensajes desde el principio, abre el monitor (115200) y aprieta **EN/RST**.
- Cierra el monitor antes de subir si PlatformIO dice que el puerto está ocupado.

---

## Fase 1 - Primer programa: parpadear un LED

Entorno `cabeza-01-blink` - programa: [`src/cabeza/01_blink`](../src/cabeza/01_blink/main.cpp)

- [x] Subir el programa y ver parpadear el LED marcado **IO2**.
- [x] Abrir el monitor serie a 115200 baudios y ver los mensajes.

Logro: sabes subir programas a la placa.

---

## Fase 2 - WiFi y control desde el celular

Entorno `cabeza-02-wifi-web` - programa: [`src/cabeza/02_wifi_web`](../src/cabeza/02_wifi_web/main.cpp)

- [x] Poner el nombre y la clave de tu WiFi en `include/secrets.h`. Ojo: la ESP32 solo usa redes de **2,4 GHz**.
- [x] Subir el programa y anotar la IP que aparece en el monitor serie.
- [x] Abrir esa IP en el navegador del celular (conectado al mismo WiFi) y encender/apagar el LED.

Logro: el robot recibe órdenes desde el celular.

> `include/secrets.h` está en `.gitignore`: tus claves nunca se suben al repo. Si falta, la compilación se detiene con el mensaje "Falta include/secrets.h".

---

## Fase 3 - Cámara

Se hizo con el ejemplo *CameraWebServer* de Arduino IDE (modelo `CAMERA_MODEL_ESP32S3_EYE`, que usa los mismos pines que esta placa). En PlatformIO, la cámara se prueba con el entorno `cabeza-05-vision`.

- [x] Abrir el ejemplo, elegir `CAMERA_MODEL_ESP32S3_EYE`, poner el WiFi y subirlo.
- [x] Probar *Start Stream* y tomar fotos.

Logro: el robot ve.

**Lo que aprendimos:** el video continuo es lento y calienta mucho la cámara, porque la cámara y el WiFi trabajan sin parar. Para el robot basta con sacar fotos cuando hacen falta (eso hace la fase 5).

---

## Fase 4 - Conectar la cabeza con el cuerpo (UART)

**Funciona:** la cabeza manda órdenes, el cuerpo enciende y apaga su LED y responde. El cuerpo por ahora es la ESP32 NodeMCU (la placa sin cámara, con un solo USB y los pines ya soldados).

Entornos `cuerpo-04-uart` y `cabeza-04-uart` - programas: [`src/cuerpo/04_uart`](../src/cuerpo/04_uart/main.cpp) y [`src/cabeza/04_uart`](../src/cabeza/04_uart/main.cpp)

### Soldar la tira de pines a la ESP32-S3

La ESP32-S3 viene sin pines. Conviene soldar las dos filas completas: la placa queda firme y después se usan más pines (audio, alimentación).

1. Desconectar el USB y sacar la cinta de la cámara (levantar la trabita negra del conector).
2. Cortar dos tramos de tira con la cantidad de agujeros de cada lado.
3. Clavar el lado largo de los pines en una protoboard (o sujetarlos con cinta o plasticina) y poner la placa encima: pines hacia abajo, cámara hacia arriba.
4. Soldar primero un pin en cada punta, revisar que la tira quede recta y después soldar el resto. Cada soldadura debe quedar como un cono pequeño y brillante.
5. Revisar con el multímetro en continuidad que no haya puentes entre pines vecinos (sobre todo 3V3, 5V y GND).
6. Volver a poner la cinta de la cámara y probar que la placa siga funcionando.

### Conectar y probar

- [x] Conectar **3 cables dupont hembra-hembra**, con las dos placas desconectadas del USB. No hace falta protoboard.

| Cabeza (ESP32-S3) | Cuerpo (ESP32 NodeMCU) | Para qué |
|---|---|---|
| GPIO 1 (TX) | GPIO 16 (RX2) | Órdenes |
| GPIO 14 (RX) | GPIO 17 (TX2) | Respuestas |
| GND | GND | Tierra común (obligatoria) |

> En algunas NodeMCU el 16 y el 17 dicen solo **16/17**, **G16/G17** o **IO16/IO17**, y están junto a los pines 5, 4, 0 y 2. **No usar los pines TX/RX (TX0/RX0)**: son los del USB.

- [x] Conectar **solo la NodeMCU**, elegir el entorno `cuerpo-04-uart` y apretar *Upload*. Si se queda en "Connecting...", mantener apretado **BOOT**.
- [x] Conectar la ESP32-S3 por el USB-C **USB** (ver "Puertos USB" en la fase 0), elegir `cabeza-04-uart` y apretar *Upload*.
  - Con las dos placas conectadas, PlatformIO puede elegir el puerto equivocado (error `This chip is ESP32, not ESP32-S3`). Solución: copiar `platformio_local.ini.example` como `platformio_local.ini` y poner el COM de cada placa (`pio device list` los muestra).
- [x] Abrir el *Serial Monitor* de `cabeza-04-uart`. Debe aparecer:

```
Cabeza lista
Cabeza -> cuerpo: LED_ON
Cuerpo -> cabeza: OK LED_ON
```

- [x] Ver que el LED de la NodeMCU se encienda y apague cada 2 segundos.

**Si no funciona:**
- Sale "Cabeza -> cuerpo" pero nunca "Cuerpo -> cabeza": intercambiar los cables de GPIO 1 y GPIO 14, y revisar el GND.
- No aparece nada: revisar que el monitor esté en el puerto de la ESP32-S3 (USB nativo) y que haya un solo monitor abierto.
- Al apretar RST de la cabeza con el monitor abierto, el LED del cuerpo dejó de parpadear. Cerrar y volver a abrir el monitor lo arregló. Sospecha (sin confirmar): el monitor queda colgado del puerto USB nativo y la cabeza se traba escribiendo en `Serial`. Revisar antes de que la cabeza funcione sin computador.
- PlatformIO sube a la placa equivocada: usar `platformio_local.ini` (fase 0) o conectar una placa a la vez.
- La NodeMCU no tiene pines 16 y 17: cambiar `UART_RX` y `UART_TX` en `src/cuerpo/04_uart/main.cpp` por otros libres (por ejemplo 25 y 26) y conectar ahí.

Logro: la cabeza le da órdenes al cuerpo. Después esas órdenes serán "avanza" o "gira".

---

## Fase 5 - IA en la nube con la cámara

Usamos **Gemini** porque tiene un nivel gratuito.

Entornos `cabeza-05-vision` (en la placa) y `cabeza-05-prueba-wokwi` (en el simulador) - programas: [`src/cabeza/05_vision`](../src/cabeza/05_vision/main.cpp) y [`src/cabeza/05_prueba_wokwi`](../src/cabeza/05_prueba_wokwi/main.cpp)

- [x] Crear la clave en [aistudio.google.com/apikey](https://aistudio.google.com/apikey) → *Create API key*. Usar el **Default project** y no agregarle facturación (así sigue gratis). Si pide restringirla, marcar solo **Generative Language API**.
- [ ] Probar en AI Studio: subir una foto y preguntar lo mismo que preguntará el robot.
- [x] (Sin placa) Probar la clave en Wokwi: crear un proyecto en [wokwi.com/projects/new/esp32-s3](https://wokwi.com/projects/new/esp32-s3), pegar el contenido de `src/cabeza/05_prueba_wokwi/main.cpp`, agregar la librería ArduinoJson y poner la clave. Resultado del 20-11-2026: *"Veo una pelota roja en el suelo contra un fondo verde y azul."*
- [ ] Poner `GEMINI_API_KEY` en `include/secrets.h` (la librería ArduinoJson la instala PlatformIO sola).
- [ ] Subir `cabeza-05-vision`, abrir la IP en el celular y apretar **¿Qué ves?**
- [ ] Anotar qué cámara dice el monitor serie (`Sensor: ...`) y si se sigue calentando.

Logro: "¿qué ves?" funciona.

> Nunca subas la API key al repo: va en `include/secrets.h`.

Ten en cuenta:
- La cámara se enciende solo para sacar la foto y se apaga después, para que no se caliente. Si la segunda foto falla, cambia `APAGAR_CAMARA_ENTRE_FOTOS` a `false`.
- En el nivel gratuito Google usa lo que envías para mejorar sus productos. No apuntes la cámara a documentos ni a personas que no quieran salir.
- Hay un límite de consultas por día. Si aparece **Error 429**, se acabó la cuota: espera o prueba al día siguiente.
- La conexión no verifica el certificado de Google (`setInsecure`). Sirve para probar en casa, pero hay que arreglarlo antes de usar el robot en otras redes.

---

## Fase 6 - Audio (cuando llegue lo pedido)

El micrófono se puede probar solo, sin esperar el amplificador.

Entorno `cabeza-06-microfono` - programa: [`src/cabeza/06_microfono`](../src/cabeza/06_microfono/main.cpp)

- [ ] Soldar los pines del micrófono (sirve para practicar antes de soldar la ESP32-S3).
- [ ] Conectar el micrófono con 6 cables dupont hembra-hembra:

| INMP441 | ESP32-S3 |
|---|---|
| VDD | 3V3 (**nunca a 5V**) |
| GND | GND |
| L/R | GND |
| SCK | GPIO 41 |
| WS | GPIO 42 |
| SD | GPIO 21 |

- [ ] Subir `cabeza-06-microfono` y aplaudir cerca del micrófono. El programa manda líneas como `>nivel:1234`: en el monitor serie se ven los números, y con la extensión **Teleplot** de VS Code (conectada al puerto de la placa, 115200) se ven como gráfico. Si no llega nada, el monitor dice qué cable revisar.
- [ ] Cuando llegue el amplificador: conectarlo según el plan de pines de abajo y probar el parlante con un tono simple.
- [ ] Probar el micrófono y el parlante juntos: grabar unos segundos y reproducirlos.

Logro: el robot escucha y habla.

---

## Fase 7 - Asistente de voz

XiaoZhi es un proyecto de ESP-IDF con su propio sistema de compilación, así que se compila aparte, fuera de este proyecto de PlatformIO.

- [ ] Instalar ESP-IDF (lo necesita XiaoZhi).
- [ ] Compilar [XiaoZhi](https://github.com/78/xiaozhi-esp32) con los pines de esta placa.
- [ ] Probar una conversación y revisar qué tan bien funciona en español.
- [ ] Conectar la IA con el cuerpo: que pueda mandar órdenes por UART.

Logro: conversas con el robot y te obedece.

---

## Plan de pines de la cabeza

La cámara, la memoria PSRAM y la ranura SD ocupan muchos pines. Quedan libres el 1, 2, 14, 21, 41, 42 y 47.

| Pin | Uso | Conecta con |
|---|---|---|
| 2 | LED de la placa (IO2) | - |
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
