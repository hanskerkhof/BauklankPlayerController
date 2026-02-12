#include "DYPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

#if defined(ESP32)
DYPlayerController::DYPlayerController(int rxPin, int txPin, int uart)
    : mySerial(uart), myDYPlayer(&mySerial) {
    mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
}
#else
DYPlayerController::DYPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin), myDYPlayer(&mySoftwareSerial) {
}
#endif

//#if defined(ESP32)
//    DYPlayerController::DYPlayerController(int rxPin, int txPin, int uart)
//        : mySerial(uart), myDYPlayer(&mySerial) {
//        mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
//    }
//#else
//    DYPlayerController::DYPlayerController(int rxPin, int txPin)
//        : mySoftwareSerial(rxPin, txPin), myDYPlayer(&mySoftwareSerial) {
//    }
//#endif

void DYPlayerController::DYPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();

    DEBUG_PRINT(DebugLevel::SETUP, "%s - mySoftwareSerial.begin(%d)", __PRETTY_FUNCTION__, 9600);

    #if !defined(ESP32)
        DEBUG_PRINT(DebugLevel::SETUP, "%s - mySoftwareSerial.begin(%d)", __PRETTY_FUNCTION__, 9600);
        mySoftwareSerial.begin(9600);
    #endif

    delay(300);  // Give some time for the serial connection to establish
    int slp = 30;
    delay(slp);

    DEBUG_PRINT(DebugLevel::SETUP, "%s - myDYPlayer.begin()", __PRETTY_FUNCTION__);

    myDYPlayer.begin();
    delay(slp);

    DEBUG_PRINT(DebugLevel::SETUP, "%s - myDYPlayer.setPlayingDevice(%d)", __PRETTY_FUNCTION__, DY::Device::Sd);

    myDYPlayer.setPlayingDevice(DY::Device::Sd);
    delay(slp);

    DEBUG_PRINT(DebugLevel::SETUP, "%s - myDYPlayer.setCycleMode(%d)", __PRETTY_FUNCTION__, DY::PlayMode::OneOff);

    myDYPlayer.setCycleMode(DY::PlayMode::OneOff);
    delay(slp);

    DEBUG_PRINT(DebugLevel::SETUP, "%s - myDYPlayer.setEq(%d)", __PRETTY_FUNCTION__, DY::Eq::Normal);

    myDYPlayer.setEq(DY::Eq::Normal);
    delay(slp);
}

void DYPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
  // Create the path for the track
  char path[11];
  sprintf(path, "/%05d.mp3", track);

  Serial.printf("  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);

  myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);
  // Call the base class for status, duration and trackName
  PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void DYPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
  Serial.printf("  !! Warning: DYPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
  playTrack(track, durationMs, trackName);

  // Create the path for the track
  char path[11];
  sprintf(path, "/%05d.mp3", track);

//  DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);

  executePlayerCommandBase(DYCmd_PlayTrack, (uint16_t)track);
  //  myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);
  // Call the base class for status, duration and trackName
  PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void DYPlayerController::stop() {
  DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ⏹️ %s - myDYPlayer.stop()", __PRETTY_FUNCTION__);
  executePlayerCommandBase(DYCmd_Stop);
//  myDYPlayer.stop();
  // Call base class for status
  PlayerController::stopSoundSetStatus();
}

void DYPlayerController::setPlayerVolume(uint8_t setPlayerVolume) {
  if (setPlayerVolume > 30) setPlayerVolume = 30;
  if (setPlayerVolume == lastSetPlayerVolume) {
    if(debug) {
      Serial.printf("  🔊 %s - playerVolume = %d, lastSetPlayerVolume = %d the same! Volume not set!\n",
        __PRETTY_FUNCTION__,
        setPlayerVolume,
        lastSetPlayerVolume);
    }
    return;
  }
  lastSetPlayerVolume = setPlayerVolume;
  if(debug) Serial.printf("  🔊 %s - Set DY Player volume to %d\n", __PRETTY_FUNCTION__, setPlayerVolume);
  executePlayerCommandBase(DYCmd_Volume, setPlayerVolume);
}

// If you prefer to pass the exact DY enum up front:
//void DYPlayerController::enableLoop()  { executePlayerCommandBase(DYCmd_SetCycle, (uint16_t)DY::PlayMode::RepeatOne); isLooping = true;  }
//void DYPlayerController::disableLoop() { executePlayerCommandBase(DYCmd_SetCycle, (uint16_t)DY::PlayMode::OneOff);     isLooping = false; }

// Either way works; but this option is a touch more resilient if library enums ever change value.
void DYPlayerController::enableLoop()  { executePlayerCommandBase(DYCmd_SetCycle, 1); isLooping = true;  }
void DYPlayerController::disableLoop() { executePlayerCommandBase(DYCmd_SetCycle, 0); isLooping = false; }

//void DYPlayerController::enableLoop() {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.println(F("DYPlayerController: Enabling loop"));
//
//    myDYPlayer.setCycleMode(DY::PlayMode::RepeatOne);
//    isLooping = true;
//}
//
//void DYPlayerController::disableLoop() {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.println(F("DYPlayerController: Disabling loop"));
//
//    myDYPlayer.setCycleMode(DY::PlayMode::OneOff);
//    isLooping = false;
//}

void DYPlayerController::setEqualizerPreset(EqualizerPreset preset) {
  uint8_t eq = 0; // DY::Eq::Normal
  switch (preset) {
    case EqualizerPreset::POP:     eq = 1; break; // DY::Eq::Pop
    case EqualizerPreset::ROCK:    eq = 2; break; // DY::Eq::Rock
    case EqualizerPreset::JAZZ:    eq = 3; break; // DY::Eq::Jazz
    case EqualizerPreset::CLASSIC: eq = 4; break; // DY::Eq::Classic
    case EqualizerPreset::BASS:    eq = 0; break; // map to Normal (DY lacks Bass)
    default:                       eq = 0; break;
  }
  executePlayerCommandBase(DYCmd_Eq, eq);
  PlayerController::setEqualizerPreset(preset);
}

//void DYPlayerController::setEqualizerPreset(EqualizerPreset preset) {
//    DY::Eq dyPreset;
//    switch (preset) {
//        case EqualizerPreset::NORMAL:
//            dyPreset = DY::Eq::Normal;
//            break;
//        case EqualizerPreset::POP:
//            dyPreset = DY::Eq::Pop;
//            break;
//        case EqualizerPreset::ROCK:
//            dyPreset = DY::Eq::Rock;
//            break;
//        case EqualizerPreset::JAZZ:
//            dyPreset = DY::Eq::Jazz;
//            break;
//        case EqualizerPreset::CLASSIC:
//            dyPreset = DY::Eq::Classic;
//            break;
//        case EqualizerPreset::BASS:
//            // DY player doesn't have a BASS preset, so we'll use Normal
//            dyPreset = DY::Eq::Normal;
//            break;
//    }
//    Serial.printf("!!!!! setEqualizerPreset: %d", dyPreset);
//    myDYPlayer.setEq(dyPreset);
//
//      DEBUG_PRINT(DebugLevel::COMMANDS, "  🎚️ DYPlayer: Set equalizer preset to %d", static_cast<int>(preset));
//
//    // call base class for status
//    PlayerController::setEqualizerPreset(preset);
//}

void DYPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any DY-specific update logic here if needed
}

void DYPlayerController::sendCommand(uint8_t type, uint16_t a, uint16_t /*b*/) {
  switch (type) {
    case DYCmd_PlayTrack: {
      char path[12];                 // "/%05u.mp3" + NUL
      snprintf(path, sizeof(path), "/%05u.mp3", (unsigned)a);

      Serial.print(F("[WIRE:DY] playSpecifiedDevicePath(SD, "));
      Serial.print(path);
      Serial.println(')');

      myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);
      break;
    }

    case DYCmd_Stop:
      Serial.println(F("[WIRE:DY] stop()"));
      myDYPlayer.stop();
      break;

    // Either way works; but this option is a touch more resilient if library enums ever change value.
    case DYCmd_SetCycle: {
      DY::PlayMode m = (a ? DY::PlayMode::RepeatOne : DY::PlayMode::OneOff);
      Serial.print(F("[WIRE:DY] setCycleMode("));
      Serial.print(a ? F("RepeatOne") : F("OneOff"));
      Serial.println(')');
      myDYPlayer.setCycleMode(m);
      break;
    }

// If you prefer to pass the exact DY enum up front:
//    case DYCmd_SetCycle: {
//      DY::PlayMode m = static_cast<DY::PlayMode>(a);
//      Serial.print(F("[WIRE:DY] setCycleMode(")); Serial.print((int)m); Serial.println(')');
//      myDYPlayer.setCycleMode(m);
//      break;
//    }

//    case DYCmd_SetCycle: {
//      DY::PlayMode m = (a ? DY::PlayMode::RepeatOne : DY::PlayMode::OneOff);
//      Serial.print(F("[WIRE:DY] setCycleMode("));
//      Serial.print(a ? F("RepeatOne") : F("OneOff"));
//      Serial.println(')');
//      myDYPlayer.setCycleMode(m);
//      break;
//    }

    case DYCmd_Volume:
      Serial.print(F("[WIRE:DY] setVolume(")); Serial.print((uint8_t)a); Serial.println(')');
      myDYPlayer.setVolume((uint8_t)a);
      break;

    case DYCmd_Eq: {
      DY::Eq eq = DY::Eq::Normal;
      switch (a) {                   // 0..5
        case 1: eq = DY::Eq::Pop;     break;
        case 2: eq = DY::Eq::Rock;    break;
        case 3: eq = DY::Eq::Jazz;    break;
        case 4: eq = DY::Eq::Classic; break;
        // 5 (Bass) -> Normal
        default: eq = DY::Eq::Normal; break;
      }
      Serial.print(F("[WIRE:DY] setEq(")); Serial.print((int)eq); Serial.println(')');
      myDYPlayer.setEq(eq);
      break;
    }

    default:
      Serial.println(F("[WIRE:DY] UNKNOWN"));
      break;
  }
}
