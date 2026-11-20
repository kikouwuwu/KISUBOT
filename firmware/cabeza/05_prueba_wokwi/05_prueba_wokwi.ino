// KISUBOT - Fase 5 (prueba sin placa): conexión con Gemini en el simulador Wokwi
//
// Sirve para probar la clave de Gemini y la conexión desde un computador
// donde no puedes conectar la placa. No usa la cámara: manda una foto de
// prueba (una pelota roja sobre pasto) que viene dentro del código.
//
// Cómo usarlo:
// 1. Entra a wokwi.com, crea un proyecto nuevo y elige la placa ESP32-S3.
// 2. Borra el código que trae y pega este.
// 3. En la pestaña "Library Manager" agrega "ArduinoJson".
// 4. Pega tu clave abajo en GEMINI_API_KEY y aprieta el botón verde (Play).
// 5. En el monitor serie debería aparecer algo como "Veo una pelota roja...".
//
// OJO: NO guardes ni compartas el proyecto con la clave puesta. Si lo
// guardas, otras personas podrían verla. Bórrala antes de guardar.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define GEMINI_API_KEY "pega-aqui-tu-clave"
#define GEMINI_MODEL "gemini-flash-lite-latest"   // el mismo que usa 05_vision
const char* PREGUNTA = "Describe en español, en una frase, lo que ves en esta foto.";

// Foto de prueba (JPEG de 128x96) ya convertida a base64
const char FOTO_PRUEBA_B64[] =
  "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAoHBwgHBgoICAgLCgoLDhgQDg0NDh0VFhEYIx8lJCIf"
  "IiEmKzcvJik0KSEiMEExNDk7Pj4+JS5ESUM8SDc9Pjv/2wBDAQoLCw4NDhwQEBw7KCIoOzs7Ozs7"
  "Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozs7Ozv/wAARCABgAIADASIA"
  "AhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQA"
  "AAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3"
  "ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWm"
  "p6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEA"
  "AwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSEx"
  "BhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElK"
  "U1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3"
  "uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwDqaKKK"
  "9o8YKKKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigAooooAKKKKACii"
  "igAooooAKKKKACiiigAooooAKKiubmG0t3uLiQRxoMsx/wA8n2rl7/xZcSSMmnxpHFyBLIpLN7gc"
  "Be/XPbIHSsK2Ip0V77O3CYDEYuVqUb+fQ62iuEj8RaxHIHN75uP4JIk2n67QD+tb2k+Jobxlt7xV"
  "t7hiFUjJSQ+x/hOex9Rgk1jSxtGo7J2fmdeKyXF4aPPJXXlqbtFFFdp44UUUUAFFFFABRWN/wkP/"
  "AE6/+RP/AK1H/CQ/9Ov/AJE/+tXF9fw/834P/IDZorG/4SH/AKdf/In/ANaj/hIf+nX/AMif/Wo+"
  "v4f+b8H/AJAbNFY3/CQ/9Ov/AJE/+tR/wkP/AE6/+RP/AK1H1/D/AM34P/IDL8WX7yXaaej/ALqN"
  "Q8oDfeYngH6AZx/tA9hV7wxDbjTfOQAzFmDsRyOnGfTGDXN6pObjWLqYps8wqwXOeNoX+amoYp5Y"
  "GLwyvGxGCUYg4/CvDr1vaVnLddPQ/SMtw8PqFOMOqu/Vmh4ght4NVZLcBRtUuijAU47fhg/jWZyC"
  "CCQQQQQcEEdCD2NFFc99bnrxjaPK9TvNCv21HSYZpGBmXKS4IzuBxkgdMjDY9xWhXH+HNTNjZTr5"
  "G8STlg2/H8Kqe3qprX/4SH/p1/8AIn/1q+gpY+jyLnlr10Z+V46EKeJqQhsmzZorG/4SH/p1/wDI"
  "n/1qP+Eh/wCnX/yJ/wDWrT6/h/5vwf8Akchs0Vjf8JD/ANOv/kT/AOtR/wAJD/06/wDkT/61H1/D"
  "/wA34P8AyAxaKKK+YEFFFFABRRRQBTv7cuonjUl0GCoH3lz/AE6j8fWqIIIyP17VtVBNZQTsXZSr"
  "n+NDg/j6/jmqufRZVnbwkPZVVePS26/4Bm0KrySCOMEsSMnGQo9T+v1xVwaXHn5ppnHdSQM/iADV"
  "qKGOBNkSBR1OO59T6mi56mL4kpKDWHi7930CGJYIViTooxk9T7n3p9FFI+Kk3JtvcKKKKQgooooA"
  "KKKKACiiigAooooAKKKKACiiigAooooAKKKKACiiigD/2Q==";

void setup() {
  Serial.begin(115200);
  Serial.println("\nKISUBOT - prueba de Gemini en Wokwi");

  // Wokwi simula una red WiFi abierta llamada Wokwi-GUEST (canal 6)
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Conectando al WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(" listo");

  // Mismo formato que usa 05_vision: foto + texto
  String cuerpo = "{\"contents\":[{\"parts\":[{\"inline_data\":{\"mime_type\":\"image/jpeg\",\"data\":\"";
  cuerpo += FOTO_PRUEBA_B64;
  cuerpo += "\"}},{\"text\":\"";
  cuerpo += PREGUNTA;
  cuerpo += "\"}]}]}";

  WiFiClientSecure cliente;
  cliente.setInsecure();
  HTTPClient http;
  http.setTimeout(30000);
  http.begin(cliente, "https://generativelanguage.googleapis.com/v1beta/models/" GEMINI_MODEL ":generateContent");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-goog-api-key", GEMINI_API_KEY);

  Serial.println("Preguntando a Gemini...");
  unsigned long t0 = millis();
  int codigo = http.POST((uint8_t*)cuerpo.c_str(), cuerpo.length());
  String respuesta = codigo > 0 ? http.getString() : String();
  http.end();
  Serial.printf("Codigo %d en %.1f s\n", codigo, (millis() - t0) / 1000.0);

  if (codigo != 200) {
    Serial.println("Algo fallo. Respuesta completa:");
    Serial.println(respuesta);
    Serial.println("400 = clave mal copiada | 403 = clave sin permiso | 404 = modelo no existe | 429 = cuota agotada");
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, respuesta)) {
    Serial.println("No pude leer la respuesta:");
    Serial.println(respuesta);
    return;
  }
  String texto;
  for (JsonVariant parte : doc["candidates"][0]["content"]["parts"].as<JsonArray>()) {
    if (parte["thought"] | false) continue;
    const char* t = parte["text"];
    if (t) texto += t;
  }
  Serial.print("Gemini dice: ");
  Serial.println(texto);
  Serial.println("Si ves la respuesta, la clave y el codigo de la fase 5 funcionan.");
}

void loop() {
  delay(1000);
}
