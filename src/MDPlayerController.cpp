#include "MDPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

using CMD = MDPlayerController::MDPlayerCommand;

static void dumpHex(const int8_t* data, size_t len) {
  Serial.print(F("[WIRE:MD] "));
  for (size_t i = 0; i < len; ++i) {
    uint8_t b = (uint8_t)data[i];
    if (b < 16) Serial.print('0');
    Serial.print(b, HEX);
    if (i + 1 < len) Serial.print(' ');
  }
  Serial.println();
}

#if defined(ESP32)
  MDPlayerController::MDPlayerController(int rxPin, int txPin)
      : mySerial(2)  // Using UART2 on ESP32
  {
      mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
      delay(300);  // Give some time for the serial connection to establish
      selectTFCard();
  }
#else
  MDPlayerController::MDPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin)
  {
      mySoftwareSerial.begin(9600);
      delay(300);
      selectTFCard();
  }
#endif

//MDPlayerController::MDPlayerController(int rxPin, int txPin)
// : mySoftwareSerial(rxPin, txPin) {
//    // Initialize SoftwareSerial in the constructor
//#if defined(ESP32)
//    : mdSerial(2)  // Using UART2 (you can use 1 or 2)
//{
//    mdSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
//#elif defined(ESP8266)
//    : mySoftwareSerial(rxPin, txPin)
//{
//    mySoftwareSerial.begin(9600);
//#endif
//
////    mySoftwareSerial.begin(9600);
//    delay(300);  // Give some time for the serial connection to establish
//
//    // Select TF card in the constructor
//    selectTFCard();
//}

void MDPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();

    // TODO Only print this when `DebugLevel::COMMANDS` is set

    #if defined(ESP32)
        if(debug) Serial.printf("  SETUP - mySerial.begin(%d)\n", 9600);
        mySerial.begin(9600);
    #elif defined(ESP8266)
        if(debug) Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);
        mySoftwareSerial.begin(9600);
    #endif
//    mySoftwareSerial.begin(9600);
    delay(300);  // (was 1000) TODO could this be a shorter delay?
}

void MDPlayerController::selectTFCard() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
if(debug) Serial.printf("  Selecting TF card\n");

    mdPlayerCommand(CMD::SEL_DEV, DEV_TF);  // select the TF card
    delay(300);
}

void MDPlayerController::enableLoop() {
  if (debug) Serial.println(F("  🔁 MDPlayerController::enableLoop"));

  // YX5300: 0x19 00 00 00 EF = start single-cycle play
  mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);
    isLooping = true;
}

void MDPlayerController::disableLoop() {
  if(debug) Serial.println(F("DYPlayerController: Disabling loop"));
  // YX5300: 0x19 00 00 01 EF = close single-cycle play
  executePlayerCommandBase(MDCmd_SetSnglCycl, 1);
//  mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
  isLooping = false;
}

// TODO Only print this when `DebugLevel::COMMANDS` is set
void MDPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
    uint16_t trackNumber = track;
    uint8_t _folder, _track;
    decodeFolderAndTrack(trackNumber, _folder, _track);
    uint16_t parameter = (_folder << 8) | _track;

    // Call base class for setting status, duration and trackName
    PlayerController::playSoundSetStatus(track, durationMs, trackName);

//    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);
    if (debug && isLooping) {
      Serial.printf("  🔁 %s - loop enabled, will repeat track\n", __PRETTY_FUNCTION__);
    }
    if(debug) Serial.printf("  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms\n", __PRETTY_FUNCTION__, track, trackName, durationMs);

    if (isLooping) {
      mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);
    }

    executePlayerCommandBase(MDCmd_PlayFolderFile, parameter);
//    mdPlayerCommand(CMD::PLAY_FOLDER_FILE, parameter);
}

void MDPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
  Serial.printf("  !! Warning: DYPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
  playTrack(track, durationMs, trackName);
}

void MDPlayerController::stop() {
  // TODO Only print this when `DebugLevel::COMMANDS` is set
  if(debug) Serial.printf("      %s - mdPlayerCommand(CMD::STOP_PLAY, 0)\n", __PRETTY_FUNCTION__);
  executePlayerCommandBase(MDCmd_Stop);
//  mdPlayerCommand(CMD::STOP_PLAY, 0);
  // Call base class for setting status
  PlayerController::stopSoundSetStatus();
}

void MDPlayerController::mdPlayerCommand(MDPlayerCommand command, uint16_t dat) {
    //    unsigned long startTime = millis();  // Start timing
    int8_t frame[8] = { 0 };
    frame[0] = 0x7e;                // starting byte
    frame[1] = 0xff;                // version
    frame[2] = 0x06;                // The number of bytes of the command without starting uint8_t and ending byte
    frame[3] = static_cast<int8_t>(command);
//    frame[3] = command;             //
    frame[4] = 0x00;                // 0x00 = no feedback, 0x01 = feedback
    frame[5] = (int8_t)(dat >> 8);  // data high byte
    frame[6] = (int8_t)(dat);       // data low byte
    frame[7] = 0xef;                // ending byte

    dumpHex(frame, sizeof(frame));   // <--- NEW: see the exact bytes

    for (uint8_t i = 0; i < 8; i++) {
        #if defined(ESP32)
            mySerial.write(frame[i]);
        #elif defined(ESP8266)
            mySoftwareSerial.write(frame[i]);
        #endif
    }
}

 void MDPlayerController::setPlayerVolume(uint8_t setPlayerVolume) {

  if (setPlayerVolume > 30) setPlayerVolume = 30;
  if (setPlayerVolume == lastSetPlayerVolume) {
    if(debug) {
    Serial.printf("  🔊 %s - playerVolume = %d, lastSetPlayerVolume = %d the same! Volume not set!\n",
      __PRETTY_FUNCTION__,
      setPlayerVolume,
      lastSetPlayerVolume);
    }
//    return;
  }
  lastSetPlayerVolume = setPlayerVolume;
  if(debug) Serial.printf("  🔊 %s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, setPlayerVolume);
  executePlayerCommandBase(MDCmd_Volume, setPlayerVolume);
  //    if (playerVolume != lastSetPlayerVolume) {
  //        mdPlayerCommand(SET_VOLUME, playerVolume);
  //        lastSetPlayerVolume = playerVolume;
  //         if(debug) Serial.printf("  🔊 %s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
  //    } else {
  //      // TODO Only print this when `DebugLevel::COMMANDS` is set
  ////       if(debug) Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
  //    }
}


void MDPlayerController::setEqualizerPreset(EqualizerPreset preset) {
  uint8_t mdPreset = 0;
  switch (preset) {
    case EqualizerPreset::POP:     mdPreset = 1; break;
    case EqualizerPreset::ROCK:    mdPreset = 2; break;
    case EqualizerPreset::JAZZ:    mdPreset = 3; break;
    case EqualizerPreset::CLASSIC: mdPreset = 4; break;
    case EqualizerPreset::BASS:    mdPreset = 5; break;
    default:                       mdPreset = 0; break;
  }
  executePlayerCommandBase(MDCmd_Eq, mdPreset);

  // call base class for status
  PlayerController::setEqualizerPreset(preset);
}

//void MDPlayerController::setEqualizerPreset(EqualizerPreset preset) {
//    uint8_t mdPreset;
//    switch (preset) {
//        case EqualizerPreset::NORMAL:
//            mdPreset = 0;
//            break;
//        case EqualizerPreset::POP:
//            mdPreset = 1;
//            break;
//        case EqualizerPreset::ROCK:
//            mdPreset = 2;
//            break;
//        case EqualizerPreset::JAZZ:
//            mdPreset = 3;
//            break;
//        case EqualizerPreset::CLASSIC:
//            mdPreset = 4;
//            break;
//        case EqualizerPreset::BASS:
//            mdPreset = 5;
//            break;
//        default:
//            mdPreset = 0; // Default to NORMAL if an unknown preset is passed
//            break;
//    }
//
//    mdPlayerCommand(SET_EQUALIZER, mdPreset);
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//if(debug) Serial.printf("MDPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
//
//    // call base class for status
//    PlayerController::setEqualizerPreset(preset);
//}

void MDPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any MD-specific update logic here if needed
}

void MDPlayerController::sendCommand(uint8_t type, uint16_t a, uint16_t /*b*/) {
  switch (type) {
    case MDCmd_PlayFolderFile: mdPlayerCommand(CMD::PLAY_FOLDER_FILE, a); break; // a = (folder<<8)|file
    case MDCmd_Stop:           mdPlayerCommand(CMD::STOP_PLAY,        0); break;
    case MDCmd_SetSnglCycl:    mdPlayerCommand(CMD::SET_SNGL_CYCL,    a); break; // if module cares (0/1)
    case MDCmd_Volume:         mdPlayerCommand(CMD::SET_VOLUME,       a); break; // a = 0..30
    case MDCmd_Eq:             mdPlayerCommand(CMD::SET_EQUALIZER,    a); break; // a = 0..5
    default: break;
  }
}
