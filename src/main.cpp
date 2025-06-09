#include <Arduino.h>
#include "../lib/ESP32-A2DP/src/BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting Bluetooth Speaker...");

  // // Use internal DAC for analog output on GPIO 25 (DAC1)
  // a2dp_sink.set_stream_output(AUDIO_OUTPUT_INTERNAL_DAC);

  // Mono output by default, comes out on GPIO 25 (DAC1)
  a2dp_sink.set_bits_per_sample(16); // use 16-bit audio
  a2dp_sink.set_volume(90);          // 0 - 100

  a2dp_sink.start("ESP32 BT Speaker");

  Serial.println("Bluetooth Speaker ready. Connect and play from phone.");
}

void loop()
{
  // Nothing needed; audio handled internally by library
}
