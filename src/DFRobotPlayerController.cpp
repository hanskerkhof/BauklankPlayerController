#include <Arduino.h>
#include "DFRobotPlayerController.h"
#include "DebugLevelManager.h"
#include "DFRobotDFPlayerMini.h"

#if defined(ESP32)
    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin, int uart)
        : mySerial(uart) {
        mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
    }
#else
    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin)
        : mySoftwareSerial(rxPin, txPin) {
    }
#endif


void DFRobotPlayerController::begin() {
  PlayerController::begin();

  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  // DFPlayer cold-boot grace time (first power-up can be slow)
  //  - 300–800 ms is often enough; 1500–2500 ms is safest with some cards.
  //  delay(1500);

  const uint8_t maxAttempts = 6;     // ~3 s total @ 500 ms backoff
  uint8_t attempt = 0;
  bool ok = false;

  while (attempt < maxAttempts && !ok) {
    attempt++;

    // Start/ensure the serial port is alive each attempt
    #if defined(ESP32)
      // mySerial already begun in ctor; if you want, call begin() again here.
      ok = myDFPlayer.begin(mySerial, /*isACK=*/true, /*doReset=*/true);
    #else
      if (!mySoftwareSerial.isListening()) {
        mySoftwareSerial.begin(9600);
      }
                                              // Use non-ACK mode on ESP8266
      ok = myDFPlayer.begin(mySoftwareSerial, /*isACK=*/false, /*doReset=*/true);
    #endif

    if (!ok) {
      // brief backoff; keep WiFi/OS happy on ESP8266
      for (uint16_t i = 0; i < 50; ++i) { delay(10); }  // 500 ms total
      Serial.printf("attempt: %d\n", attempt);
    }
  }

  if (!ok) {
    Serial.println(F("Unable to begin after retries:"));
    Serial.println(F("1. Recheck RX/TX wiring (DF TX -> MCU RX, DF RX -> MCU TX)."));
    Serial.println(F("2. Ensure SD card is inserted and files are valid."));
    // Don’t hard-spin: just return and let the rest of the app keep running.
    return;
  }

  // The internal processing takes 30–120 ms depending on the command.
  //
  // Typical command processing times:
  //
  // Command	                            Safe spacing
  // volume(), pause(), resume()	        40–80 ms
  // next(), previous()	                  70–150 ms
  // play(track) (especially in folders)	120–250 ms (SD lookup delay)
  //
  // Transmission time at 9600 baud: ~10 ms per command.
  // Processing time inside DFPlayer: 50–200 ms depending on command.
  //
  // Use:
  //  delay(120 ms) between normal commands
  //  delay(200–300 ms) after play()

  Serial.println(F("DFPlayer Mini online."));

  // Let the DFPlayer settle after reset/mount
  delay(100);
  myDFPlayer.stop();
  delay(DF_CMD_GAP_MS);
  // (optional but often helpful)
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  delay(DF_CMD_GAP_MS);
  myDFPlayer.setTimeOut(1000);
  delay(DF_CMD_GAP_MS);
}

void DFRobotPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
    int _folder = ((track - 1) / 255) + 1;
    int _track = ((track - 1) % 255) + 1;

    // DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK,
    //            "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms",
    //             __PRETTY_FUNCTION__, track, trackName, durationMs);

    Serial.printf("  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms\n", "DFRobotPlayerController::playTrack", track, trackName, durationMs);

    // myDFPlayer.playFolder(_folder, _track);
    executePlayerCommandBase(DFCmd_PlayTrack, (uint16_t)track);
    // Call the base class for status, duration and trackName
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void DFRobotPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
  Serial.printf("  !! Warning: DYPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
  playTrack(track, durationMs, trackName);
}

void DFRobotPlayerController::stop() {
    Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
    Serial.printf("      %s - myDFPlayer.stop()\n", __PRETTY_FUNCTION__);

    executePlayerCommandBase(DFCmd_Stop);
    // myDFPlayer.stop();

    // Call base class for status
    PlayerController::stopSoundSetStatus();
}

void DFRobotPlayerController::setPlayerVolume(uint8_t v) {
  if (v == lastSetPlayerVolume) return;
  lastSetPlayerVolume = v;
  executePlayerCommandBase(DFCmd_Volume, v);   // last-wins
//    if (_playerVolume != lastSetPlayerVolume) {
//      executePlayerCommandBase(DFCmd_Volume, _playerVolume);
//        // myDFPlayer.volume(_playerVolume);
//        lastSetPlayerVolume = _playerVolume;
////        Serial.printf("%s - Set DF Player volume to %d\n", __PRETTY_FUNCTION__, _playerVolume);
////    } else {
////        Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, _playerVolume);
//    }
    // delay(DF_CMD_GAP_MS);
}

void DFRobotPlayerController::enableLoop()  { executePlayerCommandBase(DFCmd_LoopOn);  isLooping = true;  }
void DFRobotPlayerController::disableLoop() { executePlayerCommandBase(DFCmd_LoopOff); isLooping = false; }

//void DFRobotPlayerController::enableLoop() {
//    delay(DF_CMD_GAP_MS);
//}
//
//void DFRobotPlayerController::disableLoop() {
//    delay(DF_CMD_GAP_MS);
//}

void DFRobotPlayerController::setEqualizerPreset(EqualizerPreset preset) {
  uint8_t eq = DFPLAYER_EQ_NORMAL;
  switch (preset) {
    case EqualizerPreset::POP:     eq = DFPLAYER_EQ_POP;     break;
    case EqualizerPreset::ROCK:    eq = DFPLAYER_EQ_ROCK;    break;
    case EqualizerPreset::JAZZ:    eq = DFPLAYER_EQ_JAZZ;    break;
    case EqualizerPreset::CLASSIC: eq = DFPLAYER_EQ_CLASSIC; break;
    case EqualizerPreset::BASS:    eq = DFPLAYER_EQ_BASS;    break;
    default:                       eq = DFPLAYER_EQ_NORMAL;  break;
  }
  executePlayerCommandBase(DFCmd_Eq, eq);
  PlayerController::setEqualizerPreset(preset);
}

//void DFRobotPlayerController::setEqualizerPreset(EqualizerPreset preset) {
//    switch(preset) {
//        case EqualizerPreset::NORMAL:
//            myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
//            break;
//        case EqualizerPreset::POP:
//            myDFPlayer.EQ(DFPLAYER_EQ_POP);
//            break;
//        case EqualizerPreset::ROCK:
//            myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
//            break;
//        case EqualizerPreset::JAZZ:
//            myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
//            break;
//        case EqualizerPreset::CLASSIC:
//            myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
//            break;
//        case EqualizerPreset::BASS:
//            myDFPlayer.EQ(DFPLAYER_EQ_BASS);
//            break;
//    }
//    delay(DF_CMD_GAP_MS);
//    // call base class for status
//    PlayerController::setEqualizerPreset(preset);
//}

void DFRobotPlayerController::sendCommand(uint8_t type, uint16_t a, uint16_t b) {
  // Wire-level: log what we are about to tell the DF library to do.
  Serial.print(F("[WIRE:DF] "));
  switch (type) {
    case DFCmd_PlayTrack: Serial.print(F("play(")); Serial.print(a); Serial.println(')'); myDFPlayer.play(a); break;
    case DFCmd_Stop:      Serial.println(F("stop()"));                                   myDFPlayer.stop();    break;
    case DFCmd_LoopOn:    Serial.println(F("loop(true)"));                                myDFPlayer.loop(true);  break;
    case DFCmd_LoopOff:   Serial.println(F("loop(false)"));                               myDFPlayer.loop(false); break;
    case DFCmd_Volume:    Serial.print(F("volume(")); Serial.print((uint8_t)a); Serial.println(')'); myDFPlayer.volume((uint8_t)a); break;
    case DFCmd_Eq:        Serial.print(F("EQ(")); Serial.print((uint8_t)a); Serial.println(')');     myDFPlayer.EQ((uint8_t)a);     break;
    default:              Serial.println(F("UNKNOWN")); break;
  }
}

void DFRobotPlayerController::update() {
    PlayerController::update(); // Call the base class update method
}
