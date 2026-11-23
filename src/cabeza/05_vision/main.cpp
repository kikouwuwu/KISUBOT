// KISUBOT - Cabeza - Fase 5: "¿Qué ves?"
//
// Entorno de PlatformIO: cabeza-05-vision
// La cabeza (ESP32-S3) saca UNA foto cuando aprietas el botón en el celular,
// se la manda a Gemini (nivel gratuito) y te muestra la respuesta.
// A diferencia de CameraWebServer, NO transmite video: la cámara se apaga
// entre fotos, así casi no se calienta.
//
// Antes de compilar: copia include/secrets.h.example como include/secrets.h
// y pon WIFI_SSID, WIFI_PASSWORD y GEMINI_API_KEY.
// La librería ArduinoJson la instala PlatformIO solo (lib_deps en platformio.ini).

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "esp_camera.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "Falta include/secrets.h: copia include/secrets.h.example como include/secrets.h y pon tus datos"
#endif

// ---------- Ajustes que puedes cambiar ----------
#define GEMINI_MODEL "gemini-flash-lite-latest"   // si da error 404 o 429, prueba "gemini-flash-latest"
#define APAGAR_CAMARA_ENTRE_FOTOS true            // true = menos calor. Si la 2a foto falla, pon false
#define RESOLUCION FRAMESIZE_VGA                  // 640x480: suficiente para la IA y rápido de enviar
#define VOLTEAR_IMAGEN 1                          // 1 = igual que CameraWebServer. Si sale al revés, pon 0
const char* PREGUNTA_POR_DEFECTO =
  "Eres los ojos de un robot pequeño. Describe en español, en máximo 2 frases, "
  "lo que ves. Si hay personas, mascotas u obstáculos cerca, dilo primero.";

const int LED_PIN = 2;
WebServer server(80);

uint8_t* ultimaFoto = nullptr;   // copia de la última foto (en PSRAM)
size_t ultimaFotoLen = 0;
bool camaraLista = false;

// ---------- Cámara ----------
bool encenderCamara() {
  if (camaraLista) return true;
  camera_config_t c = {};
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = 11; c.pin_d1 = 9;  c.pin_d2 = 8;  c.pin_d3 = 10;   // pines ESP32S3_EYE
  c.pin_d4 = 12; c.pin_d5 = 18; c.pin_d6 = 17; c.pin_d7 = 16;
  c.pin_xclk = 15; c.pin_pclk = 13; c.pin_vsync = 6; c.pin_href = 7;
  c.pin_sccb_sda = 4; c.pin_sccb_scl = 5;
  c.pin_pwdn = -1; c.pin_reset = -1;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = PIXFORMAT_JPEG;
  c.frame_size = RESOLUCION;
  c.jpeg_quality = 12;              // 10-15: buena calidad y archivo chico
  c.fb_count = 2;
  c.fb_location = CAMERA_FB_IN_PSRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("Error al iniciar la camara: 0x%x\n", err);
    return false;
  }
  sensor_t* s = esp_camera_sensor_get();
  if (s) s->set_vflip(s, VOLTEAR_IMAGEN);
  camaraLista = true;
  return true;
}

void apagarCamara() {
  if (!camaraLista) return;
  esp_camera_deinit();
  camaraLista = false;
}

const char* nombreSensor() {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return "desconocido";
  switch (s->id.PID) {
    case OV2640_PID: return "OV2640";
    case OV3660_PID: return "OV3660";
    case OV5640_PID: return "OV5640";
    default: return "otro";
  }
}

bool sacarFoto() {
  bool estabaApagada = !camaraLista;
  if (!encenderCamara()) return false;

  // Recién encendida, la cámara tarda en ajustar la luz: botamos las primeras fotos
  int descartar = estabaApagada ? 6 : 1;
  for (int i = 0; i < descartar; i++) {
    camera_fb_t* tmp = esp_camera_fb_get();
    if (tmp) esp_camera_fb_return(tmp);
    delay(80);
  }

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("No se pudo sacar la foto");
    if (APAGAR_CAMARA_ENTRE_FOTOS) apagarCamara();
    return false;
  }

  if (ultimaFoto) free(ultimaFoto);
  ultimaFoto = (uint8_t*)ps_malloc(fb->len);
  ultimaFotoLen = 0;
  if (ultimaFoto) {
    memcpy(ultimaFoto, fb->buf, fb->len);
    ultimaFotoLen = fb->len;
  }
  esp_camera_fb_return(fb);

  if (APAGAR_CAMARA_ENTRE_FOTOS) apagarCamara();
  return ultimaFotoLen > 0;
}

// ---------- Gemini ----------
String escaparJson(const String& s) {
  String r;
  r.reserve(s.length() + 10);
  for (size_t i = 0; i < s.length(); i++) {
    char ch = s[i];
    if (ch == '"' || ch == '\\') { r += '\\'; r += ch; }
    else if (ch == '\n') r += "\\n";
    else if ((uint8_t)ch < 0x20) r += ' ';
    else r += ch;
  }
  return r;
}

String explicarError(int codigo) {
  if (codigo < 0) return "Sin conexión con Gemini (error " + String(codigo) + "). Revisa el WiFi.";
  if (codigo == 400) return "Error 400: pedido rechazado. Revisa que la clave en secrets.h esté bien copiada.";
  if (codigo == 403) return "Error 403: la clave no tiene permiso. Revisa que esté restringida a 'Generative Language API'.";
  if (codigo == 404) return "Error 404: el modelo no existe. Cambia GEMINI_MODEL.";
  if (codigo == 429) return "Error 429: se acabó la cuota gratis por ahora. Espera un rato o mañana.";
  if (codigo >= 500) return "Error " + String(codigo) + ": Gemini está con problemas. Intenta de nuevo.";
  return "Error " + String(codigo) + " de Gemini (detalle en el Serial Monitor).";
}

String preguntarAGemini(const String& pregunta) {
  // Armamos el JSON: {"contents":[{"parts":[{foto},{texto}]}]}
  String foto64 = base64::encode(ultimaFoto, ultimaFotoLen);
  String cuerpo;
  cuerpo.reserve(foto64.length() + pregunta.length() + 200);
  cuerpo += "{\"contents\":[{\"parts\":[{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
  cuerpo += foto64;
  cuerpo += "\"}},{\"text\":\"";
  cuerpo += escaparJson(pregunta);
  cuerpo += "\"}]}]}";
  foto64 = String();  // libera memoria

  WiFiClientSecure cliente;
  cliente.setInsecure();  // prototipo: no verifica el certificado de Google
  HTTPClient http;
  http.setConnectTimeout(10000);
  http.setTimeout(30000);
  String url = "https://generativelanguage.googleapis.com/v1beta/models/" GEMINI_MODEL ":generateContent";
  if (!http.begin(cliente, url)) return "No pude abrir la conexión con Gemini.";
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-goog-api-key", GEMINI_API_KEY);

  int codigo = http.POST((uint8_t*)cuerpo.c_str(), cuerpo.length());
  String respuesta = codigo > 0 ? http.getString() : String();
  http.end();

  if (codigo != 200) {
    Serial.printf("Gemini respondio %d:\n%s\n", codigo, respuesta.c_str());
    return explicarError(codigo);
  }

  JsonDocument doc;
  DeserializationError e = deserializeJson(doc, respuesta);
  if (e) return String("No entendí la respuesta de Gemini: ") + e.c_str();

  // La respuesta viene en candidates[0].content.parts[].text
  String texto;
  for (JsonVariant parte : doc["candidates"][0]["content"]["parts"].as<JsonArray>()) {
    if (parte["thought"] | false) continue;
    const char* t = parte["text"];
    if (t) texto += t;
  }
  texto.trim();
  if (texto.isEmpty()) {
    const char* motivo = doc["candidates"][0]["finishReason"] | "sin motivo";
    return String("Gemini no respondió texto (") + motivo + ").";
  }
  return texto;
}

String mirarYPreguntar(String pregunta) {
  pregunta.trim();
  if (pregunta.isEmpty()) pregunta = PREGUNTA_POR_DEFECTO;
  digitalWrite(LED_PIN, HIGH);
  unsigned long t0 = millis();

  String resultado;
  if (!sacarFoto()) {
    resultado = "No pude sacar la foto. Aprieta EN/RST y prueba de nuevo.";
  } else {
    Serial.printf("Foto de %u bytes. Enviando a Gemini...\n", (unsigned)ultimaFotoLen);
    resultado = preguntarAGemini(pregunta);
  }

  Serial.printf("Respuesta (%.1f s): %s\n", (millis() - t0) / 1000.0, resultado.c_str());
  Serial.printf("Temperatura del chip: %.0f C\n", temperatureRead());
  digitalWrite(LED_PIN, LOW);
  return resultado;
}

// ---------- Página web ----------
const char PAGINA[] = R"HTML(<!DOCTYPE html>
<html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>KISUBOT</title>
<style>
body{font-family:sans-serif;max-width:480px;margin:20px auto;padding:0 12px;background:#111;color:#eee}
button{width:100%;padding:18px;font-size:22px;border:0;border-radius:12px;background:#2e7d32;color:#fff}
button:disabled{background:#555}
input{width:100%;box-sizing:border-box;padding:10px;font-size:16px;margin:10px 0;border-radius:8px;border:0}
#r{font-size:20px;line-height:1.4;margin:16px 0;min-height:2em}
img{width:100%;border-radius:12px;display:none}
small{color:#999}
</style></head><body>
<h2>KISUBOT</h2>
<input id="q" placeholder="Pregunta opcional (ej: ¿hay algo en el suelo?)">
<button id="b" onclick="ver()">¿Qué ves?</button>
<div id="r"></div>
<img id="f" alt="última foto">
<p><small id="t"></small></p>
<script>
async function temp(){try{const r=await fetch('/temp');t.textContent='Temperatura del chip: '+await r.text()+' °C';}catch(e){}}
async function ver(){
  b.disabled=true;r.textContent='Mirando y pensando...';
  try{
    const res=await fetch('/ver?p='+encodeURIComponent(q.value));
    r.textContent=await res.text();
    f.src='/foto.jpg?'+Date.now();f.style.display='block';
  }catch(e){r.textContent='Se perdió la conexión con el robot.';}
  b.disabled=false;temp();
}
temp();
</script></body></html>)HTML";

void manejarInicio() {
  server.send(200, "text/html; charset=utf-8", PAGINA);
}

void manejarVer() {
  String r = mirarYPreguntar(server.arg("p"));
  server.send(200, "text/plain; charset=utf-8", r);
}

void manejarFoto() {
  if (!ultimaFoto || ultimaFotoLen == 0) {
    server.send(404, "text/plain", "Todavia no hay foto");
    return;
  }
  server.setContentLength(ultimaFotoLen);
  server.send(200, "image/jpeg", "");
  server.sendContent((const char*)ultimaFoto, ultimaFotoLen);
}

void manejarTemp() {
  server.send(200, "text/plain", String(temperatureRead(), 0));
}

// ---------- Inicio ----------
void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("\nKISUBOT - Fase 5: vision con Gemini");

  if (!psramFound()) {
    Serial.println("OJO: no hay PSRAM. Revisa board_build.arduino.memory_type = qio_opi en platformio.ini.");
  }

  // Prueba rápida de la cámara al arrancar
  if (encenderCamara()) {
    Serial.printf("Camara OK. Sensor: %s\n", nombreSensor());
    if (APAGAR_CAMARA_ENTRE_FOTOS) apagarCamara();
  } else {
    Serial.println("La camara no inicio. Revisa la cinta (flex) y reinicia.");
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando al WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Listo. Abre en el celular: http://");
  Serial.println(WiFi.localIP());
  Serial.println("Tambien puedes escribir una pregunta aqui en el Serial Monitor y apretar Enter.");

  server.on("/", manejarInicio);
  server.on("/ver", manejarVer);
  server.on("/foto.jpg", manejarFoto);
  server.on("/temp", manejarTemp);
  server.begin();
}

void loop() {
  server.handleClient();

  // Prueba sin celular: escribe en el Serial Monitor y aprieta Enter
  if (Serial.available()) {
    String linea = Serial.readStringUntil('\n');
    mirarYPreguntar(linea);
  }
}
