#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "wifi-data.hpp"

#define AUDIO_PIN 25      // DAC1
#define SAMPLE_RATE 44100 // sample rate in Hz
#define BUFFER_SIZE 256   // bytes per chunk

AsyncWebServer server(80);
File audioFile;
volatile bool playing = false;

hw_timer_t *timer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Playback buffer
uint8_t buffer[BUFFER_SIZE];
size_t bufLen = 0;
size_t bufIndex = 0;

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  if (playing && bufIndex < bufLen)
  {
    uint8_t sample = buffer[bufIndex++];
    dacWrite(AUDIO_PIN, sample);
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void startPlayback()
{
  if (!SPIFFS.exists("/audio.wav"))
    return;
  audioFile = SPIFFS.open("/audio.wav", FILE_READ);
  if (!audioFile)
    return;
  // Skip WAV header (assumes 44 bytes)
  audioFile.seek(44);

  // load first chunk
  bufLen = audioFile.read(buffer, BUFFER_SIZE);
  bufIndex = 0;

  playing = true;
}

void setup()
{
  Serial.begin(115200);
  // Mount SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" verbunden!");
  Serial.println(WiFi.localIP());

  // Configure DAC pin
  dacWrite(AUDIO_PIN, 128); // mid-level

  // Timer interrupt for playback
  timer = timerBegin(0, 80, true); // prescaler 80 => 1MHz
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000 / SAMPLE_RATE, true);
  timerAlarmEnable(timer);

  // Web server: upload form
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  // File upload handler
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    request->send(200, "text/plain", "Upload complete!");
    // Start playback
    startPlayback(); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
    if (!index) {
      // First chunk, open file
      if (SPIFFS.exists("/audio.wav")) SPIFFS.remove("/audio.wav");
      File file = SPIFFS.open("/audio.wav", FILE_WRITE);
      file.close();
    }
    File file = SPIFFS.open("/audio.wav", FILE_APPEND);
    if (file) {
      file.write(data, len);
      file.close();
    } });

  // Serve index.html and audio player
  server.serveStatic("/index.html", SPIFFS, "/index.html");

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  if (playing && bufIndex >= bufLen)
  {
    // refill buffer
    bufLen = audioFile.read(buffer, BUFFER_SIZE);
    bufIndex = 0;
    if (bufLen == 0)
    {
      // finished
      audioFile.close();
      playing = false;
    }
  }
}
