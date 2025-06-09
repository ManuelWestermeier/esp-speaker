#include <Arduino.h>
#include "../lib/ESP32-A2DP/src/BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

// Event callback for debugging
void bt_event_callback(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
  switch (event)
  {
  case ESP_A2D_CONNECTION_STATE_EVT:
    Serial.printf("[BT] Connection state: %s\n",
                  param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED ? "CONNECTED" : param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED ? "DISCONNECTED"
                                                                                                                                                               : "UNKNOWN");
    break;
  case ESP_A2D_AUDIO_STATE_EVT:
    Serial.printf("[BT] Audio state: %s\n",
                  param->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED ? "STARTED" : param->audio_stat.state == ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND ? "SUSPENDED"
                                                                                   : param->audio_stat.state == ESP_A2D_AUDIO_STATE_STOPPED          ? "STOPPED"
                                                                                                                                                     : "UNKNOWN");
    break;
  default:
    Serial.printf("[BT] Event %d received\n", event);
    break;
  }
}

// Metadata callback (track info)
void audio_metadata_callback(const char *type, const char *value)
{
  Serial.printf("[META] %s: %s\n", type, value);
}

void setup()
{
  Serial.begin(115200);
  delay(1000); // allow time for Serial

  Serial.println("\n=== ESP32 Bluetooth Speaker Booting ===");

  // Setup stream output (internal DAC on GPIO 25)
  // Serial.println("[CONFIG] Using internal DAC (GPIO 25)");
  // a2dp_sink.set_stream_output(AUDIO_OUTPUT_INTERNAL_DAC);

  // Set audio resolution
  a2dp_sink.set_bits_per_sample(16); // 16-bit audio
  Serial.println("[CONFIG] Bits per sample: 16");

  // Set volume
  a2dp_sink.set_volume(100);
  Serial.println("[CONFIG] Volume: 100");

  // Register event and metadata callbacks
  // a2dp_sink.set_on_audio_state_changed(bt_event_callback);
  // a2dp_sink.set_avrc_metadata_callback(audio_metadata_callback);

  // Start Bluetooth sink
  a2dp_sink.start("ESP32 BT Speaker");
  Serial.println("[BOOT] Bluetooth speaker ready.");
  Serial.println("[BOOT] Pair with 'ESP32 BT Speaker' and start playback.");
}

void loop()
{
  // Nothing needed in loop
}
