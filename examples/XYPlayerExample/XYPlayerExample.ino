#include <arduino.h>
#include "XYPlayerController.h"

#if defined(ESP32)
// RX = 16, TX = 17, UART2
XYPlayerController player(16, 17, 2);
#else
XYPlayerController player(D1, D2);  // TX, RX for SoftwareSerial
#endif

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  Serial.println(__FILE__);
  Serial.println("Compiled " __DATE__ " of " __TIME__);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  player.begin();
  player.setVolume(15);
  player.playTrack(1, 5000, "Test track");
}

void loop() {
  player.update();
}
