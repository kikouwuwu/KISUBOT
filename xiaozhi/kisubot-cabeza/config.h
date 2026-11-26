// KISUBOT - Cabeza para XiaoZhi
// ESP32-S3 N16R8 con cámara OV5640 (mismos pines de cámara que bread-compact-wifi-s3cam).
// Los pines siguen el "Plan de pines de la cabeza" de docs/plan-cabeza.md del repo KISUBOT.
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ---------- Audio: INMP441 (micrófono) + MAX98357A (amplificador) ----------
// Modo duplex: el micrófono y el amplificador comparten BCLK y WS, así que
// usan UN solo reloj. Por eso entrada y salida van a la misma frecuencia:
// XiaoZhi convierte el micrófono de 24 kHz a 16 kHz por software.
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_41   // SCK del INMP441 y BCLK del MAX98357A
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_42   // WS del INMP441 y LRC del MAX98357A
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_21   // SD del INMP441
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_47   // DIN del MAX98357A

// ---------- LED y botón ----------
#define BUILTIN_LED_GPIO  GPIO_NUM_2      // LED "IO2" de la placa (LED común, no RGB)
#define BOOT_BUTTON_GPIO  GPIO_NUM_0

// ---------- UART hacia el cuerpo (ESP32 NodeMCU) ----------
// Reservados para la conexión con el cuerpo (fase 4). Todavía no se usan aquí.
#define CUERPO_UART_TX GPIO_NUM_1         // -> GPIO 16 de la NodeMCU
#define CUERPO_UART_RX GPIO_NUM_14        // <- GPIO 17 de la NodeMCU

// ---------- Cámara OV5640 ----------
#define CAMERA_PIN_D0    GPIO_NUM_11
#define CAMERA_PIN_D1    GPIO_NUM_9
#define CAMERA_PIN_D2    GPIO_NUM_8
#define CAMERA_PIN_D3    GPIO_NUM_10
#define CAMERA_PIN_D4    GPIO_NUM_12
#define CAMERA_PIN_D5    GPIO_NUM_18
#define CAMERA_PIN_D6    GPIO_NUM_17
#define CAMERA_PIN_D7    GPIO_NUM_16
#define CAMERA_PIN_XCLK  GPIO_NUM_15
#define CAMERA_PIN_PCLK  GPIO_NUM_13
#define CAMERA_PIN_VSYNC GPIO_NUM_6
#define CAMERA_PIN_HREF  GPIO_NUM_7
#define CAMERA_PIN_SIOC  GPIO_NUM_5
#define CAMERA_PIN_SIOD  GPIO_NUM_4
#define CAMERA_PIN_PWDN  GPIO_NUM_NC      // la placa no tiene pin para cortarle la energía
#define CAMERA_PIN_RESET GPIO_NUM_NC
#define XCLK_FREQ_HZ     20000000

// Sin pantalla: los pines 19 y 20 quedan para el USB nativo (subir programas y monitor).

#endif // _BOARD_CONFIG_H_
