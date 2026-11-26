# Robot autónomo con ESP32 e IA en la nube

## Estado actual

Estamos armando la **cabeza** (ESP32-S3 con cámara). Los motores vienen después.

- Funcionan en la placa: LED, página web por WiFi y cámara (fases 0 a 3).
- Comunicación cabeza-cuerpo (fase 4): funciona.
- "¿Qué ves?" con Gemini (fase 5): funciona en la placa con la cámara.

**Lista de tareas:** [docs/plan-cabeza.md](docs/plan-cabeza.md)

### Cómo se programa

El firmware se programa con **[PlatformIO](https://platformio.org/)** en VS Code. Todo es un solo proyecto (`platformio.ini` en la raíz) y cada programa es un **entorno**.

```
platformio.ini                 placas, memoria, librerías y entornos
platformio_local.ini.example   ajustes de tu computador (puertos), opcional
include/secrets.h.example      plantilla de claves: copiar como include/secrets.h
src/cabeza/<programa>/         programas de la ESP32-S3
src/cuerpo/<programa>/         programas de la ESP32 NodeMCU
docs/                          planes y guías
```

Primeros pasos:

1. Instalar VS Code y la extensión **PlatformIO IDE**.
2. Abrir la carpeta del repo en VS Code.
3. Copiar `include/secrets.h.example` como `include/secrets.h` y poner tus datos (no se sube al repo).
4. Elegir el entorno en la barra inferior de PlatformIO y usar *Build*, *Upload* y *Serial Monitor*.

Desde la terminal: `pio run -e cabeza-01-blink -t upload` y `pio device monitor -e cabeza-01-blink`.

| Entorno | Placa | Qué hace |
|---|---|---|
| `cabeza-01-blink` | ESP32-S3 | Hace parpadear el LED (primer programa) |
| `cabeza-02-wifi-web` | ESP32-S3 | Página web para controlar el LED desde el celular |
| `cabeza-04-uart` | ESP32-S3 | Manda órdenes al cuerpo por UART |
| `cuerpo-04-uart` | ESP32 NodeMCU | Recibe órdenes de la cabeza y responde |
| `cabeza-05-vision` | ESP32-S3 | Saca una foto, se la manda a Gemini y muestra qué ve |
| `cabeza-05-prueba-wokwi` | Simulador Wokwi | Prueba la clave de Gemini sin la placa |
| `cabeza-06-microfono` | ESP32-S3 | Muestra cuánto sonido escucha el micrófono |

Más detalles (puertos USB, drivers, qué hacer si falla la subida) en la fase 0 del [plan](docs/plan-cabeza.md).

---

## Descripción del proyecto

Robot de dos ruedas (2WD) que:

- **Anda solo**: detecta obstáculos y bordes, y decide hacia dónde moverse.
- **Conversa por voz**: escucha con un micrófono, responde por un parlante y usa un modelo de lenguaje en la nube.
- **Ve con una cámara**: la IA puede analizar lo que tiene enfrente ("busca la pelota", "¿qué ves?").
- **Recibe órdenes desde el celular**: mediante una página web del propio robot o un bot de Telegram.

### Cómo está organizado

El robot usa dos placas que se reparten el trabajo:

| Parte | Placa | Qué hace |
|---|---|---|
| **Cabeza** | ESP32-S3 N16R8 con cámara | WiFi, conexión con la IA, cámara, micrófono y parlante |
| **Cuerpo** | ESP32 NodeMCU (ESP-32S) | Motores, sensores y "reflejos" (frenar, esquivar, no caerse) |

Las placas se comunican por **UART** (2 cables). La cabeza manda órdenes generales al cuerpo ("avanza 50 cm", "gira 90°", "explora") y el cuerpo avisa cuando encuentra un obstáculo.

**Por qué dividirlo así:**
- La cámara ocupa muchos pines de la ESP32-S3; los que quedan alcanzan para el audio y la conexión con el cuerpo, pero no para todo lo demás.
- La IA en la nube tarda un par de segundos en responder. Los reflejos los resuelve el cuerpo en milisegundos, sin depender de internet: si se corta el WiFi, el robot igual frena.

### Software

- **Cuerpo**: PlatformIO con el framework Arduino.
- **Cabeza**: por ahora, programas de prueba en PlatformIO con el framework Arduino. Después, [XiaoZhi](https://github.com/78/xiaozhi-esp32), un proyecto de código abierto para conversar por voz con modelos de lenguaje. Usa el protocolo MCP, que permite que la IA dé órdenes al robot. Se compila con ESP-IDF (más complejo que Arduino).
  - Con el servidor oficial, la voz pasa por servidores de terceros. También se puede instalar un servidor propio.
  - Falta comprobar qué tan bien conversa en español.

---

## Componentes

### Lo que ya tienes

| Componente | Para qué sirve |
|---|---|
| **ESP32-S3 N16R8 con cámara** (2 USB-C) | Cabeza del robot. La cámara ya viene conectada (OV5640, confirmada por el programa de la fase 5). Pines libres: 1, 2, 14, 21, 41, 42 y 47 (más 38, 39 y 40 si no usas tarjeta SD). |
| **ESP32 NodeMCU (ESP-32S)** | Cuerpo del robot. Controla motores y sensores. |
| **Sensor infrarrojo de obstáculos** (MH-B, LM393) | Mirando al suelo detecta bordes y escaleras. Mirando a un lado detecta obstáculos cercanos (aprox. 2 a 30 cm). Su sensibilidad se ajusta con el potenciómetro azul. |
| **Conversor de nivel TXS0108E** | Convierte señales entre 5 V y 3,3 V. No hace falta con esta lista; queda de reserva. |
| **Cautín, protoboard y multímetro** | Armado, pruebas y medición. El multímetro es necesario para ajustar el regulador LM2596. |
| ~~Placa de sensor de humedad o lluvia~~ | Sin la sonda no sirve para este robot. |

### Comprar en AliExpress

| Componente | Cant. | Para qué sirve | Link |
|---|---|---|---|
| **Micrófono INMP441** | 1 | Micrófono digital (I2S) para que la cabeza escuche tu voz. | [ver](https://www.aliexpress.us/i/3256809766096721.html) |
| **Amplificador MAX98357A** | 1 | Convierte el audio digital de la ESP32-S3 en sonido y mueve el parlante (hasta 3 W). | [ver](https://www.aliexpress.us/item/3256810340465738.html) |
| **Parlante 40 mm, 3 W, 4 Ω** | 1 | La voz del robot. | [ver](https://www.aliexpress.us/item/3256811726937954.html) |
| **Driver de motores TB6612FNG** | 1 | Controla velocidad y sentido de los 2 motores. El ESP32 no puede alimentar motores directamente. Elegir "TB6612FNG weld". | [ver](https://www.aliexpress.us/item/3256805570351374.html) |
| **Sensor láser VL53L0X** | 1 | Mide la distancia al frente con precisión (aprox. 5 cm a 1,2 m). Se conecta por I2C. | [ver](https://www.aliexpress.us/item/3256808474068448.html) |
| **Giroscopio MPU6050** | 1 | Mide giros e inclinación: permite avanzar derecho y girar ángulos exactos. Se conecta por I2C (comparte cables con el VL53L0X). | [ver](https://www.aliexpress.us/item/3256807345627856.html) |
| **Regulador LM2596** | 1 | Baja los 7,4 V de las baterías a 5 V para las placas y el amplificador. | [ver](https://www.aliexpress.us/item/2251832535193001.html) |

### Tracción: motores y chasis (armado por ti)

#### Opciones de motor

Necesitas **2 motores** (uno por rueda). Todas las opciones funcionan con el driver TB6612FNG.

| Opción | Cómo es | Ventajas | Desventajas | Link |
|---|---|---|---|---|
| **A. Motor TT amarillo** (3–6 V, 1:48, ~200 RPM, doble eje) | Caja reductora de plástico, el más usado en robots de este tipo. | Barato, fácil de montar, muchas ruedas y soportes compatibles. | Sin encoder: no sabe cuánto avanzó (se puede agregar un sensor de ranura). | [Motor](https://www.aliexpress.us/item/3256808497240025.html) · [Combo 2 motores + 2 ruedas](https://www.aliexpress.us/item/2261800123340069.html) |
| **B. Motor TT con encoder Hall** (6 V, 1:48), **recomendado** | Igual al TT, pero con un sensor que cuenta las vueltas. | Mismo montaje y ruedas que el TT; mide distancia y velocidad con precisión. | Algo más caro; ocupa 2 pines extra por motor. | [Ver opciones](https://es.aliexpress.com/w/wholesale-motor-encoder.html) (buscar "Hall Encoder TT Motor AB Phase") |
| **C. Motor N20 con encoder** (6 V, engranajes metálicos) | Motor muy pequeño. Este aviso trae rueda de 34 mm, soporte y tornillos. | Compacto, preciso y con engranajes de metal. | Ruedas chicas: le cuesta en alfombras; menos fuerza. Elegir 300 RPM. | [Ver](https://www.aliexpress.us/item/3256801456325743.html) |
| **D. Motor JGA25-370 con encoder** (6 V, engranajes metálicos) | Motor más grande y robusto. | Mucha fuerza, ideal si el robot queda pesado. Hay variante con rueda de 65 mm ("65mm Set Encoder"). | Más caro y pesado; consume más corriente, cerca del límite del TB6612. Elegir 6 V y 170 RPM. | [Ver](https://www.aliexpress.us/item/3256806026933051.html) |

**Recomendación:** opción B. Se arma igual que un TT común y te da encoders, que sirven para que el robot sepa cuánto avanzó.

#### Piezas del chasis (para opciones A y B)

| Pieza | Cant. | Para qué sirve | Link |
|---|---|---|---|
| **Rueda de 65 mm para motor TT** | 2 | Ruedas motrices (no hace falta si compras el combo). | [Ver](https://www.aliexpress.us/item/3256811983064161.html) |
| **Rueda loca de bolita** | 1 | Tercera rueda que gira libre y equilibra el robot. | [Ver opciones](https://www.aliexpress.com/w/wholesale-robot-ball-caster.html) |
| **Soporte en "T" para motor TT** (con tornillos M3×30) | 4 (2 por motor) | Fijar los motores a la base. | [Ver](https://www.aliexpress.com/item/32438528865.html) |
| **Kit de separadores M3** | 1 | Fijar las placas y armar un segundo piso. Elegir variante M3. | [Ver](https://www.aliexpress.com/item/1005005263472533.html) |
| **Interruptor KCD11** | 1 | Encender y apagar el robot. | [Ver](https://www.aliexpress.us/item/2251832746835412.html) |
| **Condensadores cerámicos 100 nF (104)** | 2 | Se sueldan en los terminales de cada motor para reducir el ruido eléctrico. | [Ver](https://www.aliexpress.us/item/2251832532758555.html) |
| **Sensor de velocidad de ranura LM393** (solo opción A, opcional) | 2 | Cuenta las vueltas de la rueda con un disco ranurado. | [Ver](https://www.aliexpress.us/item/3256804079901304.html) |
| **Base**: acrílico, MDF o PVC espumado de 3–5 mm | ~15×20 cm | Estructura del robot. Ferretería o tienda de plásticos. | — |
| **Cable delgado 22–24 AWG** | — | Conectar los motores al driver. | — |

### Comprar en Viña del Mar

| Componente | Cant. | Para qué sirve |
|---|---|---|
| **Baterías 18650** (ej. NCR18650GA, 3450 mAh) | 2 | Energía del robot (7,4 V en serie). |
| **Cargador de baterías 18650** | 1 | Cargarlas de forma segura. |
| **Portapilas 2×18650** | 1 | Sostener y conectar las baterías. En Global: MH-620. |
| **Sensor infrarrojo IR-SEN** (igual al que tienes) | 1 a 3 | Con 1 más tienes 2 para detectar bordes; con 3, además 2 para los costados. En Global. |

Tienda: Global Chile Electrónica, globalelectronica.cl (confirmar stock por WhatsApp).

---

## Alimentación

```
2 × 18650 (7,4 V) ──┬──► Driver TB6612FNG (VM) ──► Motores
                    │
                    └──► LM2596 (ajustado a 5 V) ──┬──► ESP32-S3 (pin 5V)
                                                   ├──► ESP32 NodeMCU (pin 5V)
                                                   └──► MAX98357A (VIN)

3,3 V (de las placas) ──► INMP441, VL53L0X, MPU6050, sensores IR, lógica del TB6612
```

- **Ajusta el LM2596 a 5 V con el multímetro antes de conectarle cualquier cosa.** Viene de fábrica con un voltaje cualquiera y puede quemar las placas.
- **Todas las tierras (GND) van unidas**: baterías, regulador, driver y ambas placas.
- Alimenta los sensores con 3,3 V para que sus señales sean compatibles con los ESP32.

---

## Etapas sugeridas

1. Armar el chasis y mover los motores con el ESP32 NodeMCU desde PlatformIO.
2. Agregar el VL53L0X y los infrarrojos: que el robot avance, frene ante obstáculos y no se caiga.
3. Agregar el MPU6050 para avanzar derecho y girar ángulos exactos.
4. Conectar ambas placas por UART y mandar órdenes simples desde la ESP32-S3.
5. Cargar XiaoZhi en la ESP32-S3 con el micrófono y el parlante.
6. Conectar la IA con el cuerpo (vía MCP) y sumar la cámara.

---

## Notas de compra

- Los avisos de AliExpress mostraban envío gratis al buscarlos, pero esa información era de la versión para EE.UU. Antes de pagar, revisa el carrito: si algo cobra envío, cámbialo por un producto con etiqueta **Choice**.
- Las compras en AliExpress pagan IVA en Chile.
- Como sabes soldar, puedes elegir variantes "sin soldar" (unwelded), que suelen ser más baratas.
