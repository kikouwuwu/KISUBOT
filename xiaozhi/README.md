# XiaoZhi en la cabeza de KISUBOT (fase 7)

[XiaoZhi](https://github.com/78/xiaozhi-esp32) es el asistente de voz que irá en la cabeza (ESP32-S3).
No se compila con PlatformIO: usa **ESP-IDF v6.0.1 o superior** (recomiendan la rama 6.1) y vive en
su propio repo. Aquí guardamos solo nuestra placa y un script que la instala en el clon de XiaoZhi.

```
xiaozhi/kisubot-cabeza/     la placa: pines, audio, LED y cámara
xiaozhi/instalar-placa.ps1  copia la placa al clon de XiaoZhi y la registra
```

## Por qué una placa propia

Partimos de `bread-compact-wifi-s3cam` (mismos pines de cámara), pero esa placa no sirve tal cual:

| Problema en `bread-compact-wifi-s3cam` | En `kisubot-cabeza` |
|---|---|
| Usa los pines 19 y 20 para una pantalla: son los del USB nativo | Sin pantalla; el USB nativo sigue funcionando |
| Micrófono en GPIO 1 y una lámpara de prueba en GPIO 14 (la UART al cuerpo) | Audio en 41, 42, 21 y 47, como dice el plan de pines |
| LED RGB en GPIO 48 | LED común en GPIO 2, en el canal PWM 1 (el 0 es de la cámara) |
| Cámara encendida todo el tiempo | Se enciende solo para cada foto y la OV5640 queda en reposo (si no, se calienta) |

La orientación de la cámara (volteada, como en la fase 5) está en el código de la placa y no en
`config.json`: `build.py` solo acepta `camera_vflip` si encuentra `new Esp32Camera`, y aquí la
cámara se crea con `make_unique` para poder apagarla.

El audio va en modo **duplex** (micrófono y amplificador comparten BCLK y WS). Con un solo reloj, la
entrada y la salida se declaran a 24 kHz y XiaoZhi baja el micrófono a 16 kHz por software.

## Preparar el computador (una vez)

1. Instalar el instalador de Espressif y ESP-IDF solo para el ESP32-S3:
   ```powershell
   winget install --id Espressif.EIM-CLI -e
   eim install -i v6.1.1 -t esp32s3 -p C:\Espressif -n true -a true --do-not-track true
   ```
2. Clonar XiaoZhi fuera de este repo:
   ```powershell
   git clone --depth 1 https://github.com/78/xiaozhi-esp32.git $env:USERPROFILE\esp\xiaozhi-esp32
   ```
3. Instalar nuestra placa en el clon (desde la raíz de KISUBOT):
   ```powershell
   powershell -ExecutionPolicy Bypass -File xiaozhi\instalar-placa.ps1
   ```
   Si después actualizas el clon de XiaoZhi (`git pull`), vuelve a correr el script.

## Compilar y subir

En una terminal de ESP-IDF (el instalador deja un acceso directo en el escritorio):

```powershell
$env:IDF_COMPONENT_CACHE_PATH = "C:\idfc"   # ver "Rutas largas en Windows"
cd $env:USERPROFILE\esp\xiaozhi-esp32
python scripts/build.py kisubot-cabeza --language es-ES
```

La primera compilación tarda varios minutos (descarga ~75 componentes). Resultado del 26-11-2026:
`xiaozhi.bin` de 2,2 MB, 45 % libre en la partición.

Para subirlo y ver los mensajes:

```powershell
idf.py -p COM3 flash          # por el USB-C "USB" (nativo)
idf.py -p COM4 monitor        # por el USB-C "UART": XiaoZhi escribe sus mensajes ahí
```

> **Ojo:** a diferencia de los programas de PlatformIO, XiaoZhi manda sus mensajes por el puerto
> **UART** (chip CH343), no por el USB nativo. Para ver el monitor hay que conectar el cable al
> USB-C "UART" (o usar los dos cables). Los números COM pueden ser otros en tu computador:
> `idf.py` los lista si no pones `-p`.

La palabra para despertarlo es **"Hi ESP"** (en inglés): es la que `build.py` elige para todo
idioma que no sea chino. También se activa apretando el botón **BOOT**.

### Rutas largas en Windows

Si la compilación falla con `FileNotFoundError` al descomprimir un componente (por ejemplo
`esp_video`), es el límite de 260 caracteres de Windows: la caché de componentes está en una ruta
larga (`AppData\Local\Espressif\ComponentManager\Cache`). Solución sin permisos de administrador:
la variable `IDF_COMPONENT_CACHE_PATH` apuntando a una carpeta corta, como arriba. Para no tener
que ponerla cada vez: *Configuración → Variables de entorno del usuario*.

Si falla por "command line too long", ver la sección *Windows: command line too long* de
`docs/custom-board.md` en el repo de XiaoZhi.

## Ojo con la privacidad

Con el servidor oficial de XiaoZhi, **la voz y las fotos pasan por servidores de terceros**. No lo
uses cerca de conversaciones o documentos privados. También se puede instalar un servidor propio.
