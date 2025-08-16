#include "MDPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

using CMD = MDPlayerController::MDPlayerCommand;

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
        Serial.printf("  SETUP - mySerial.begin(%d)\n", 9600);
        mySerial.begin(9600);
    #elif defined(ESP8266)
        Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);
        mySoftwareSerial.begin(9600);
    #endif
//    mySoftwareSerial.begin(9600);
    delay(300);  // (was 1000) TODO could this be a shorter delay?
}

void MDPlayerController::selectTFCard() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("  Selecting TF card\n");

    mdPlayerCommand(CMD::SEL_DEV, DEV_TF);  // select the TF card
    delay(300);
}

void MDPlayerController::enableLoop() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.println(F("MDPlayerController: Enabling loop"));

    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = true;
}

void MDPlayerController::disableLoop() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.println(F("DYPlayerController: Disabling loop"));

    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = false;
}

void MDPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    uint16_t trackNumber = track;
    uint8_t _folder, _track;
    decodeFolderAndTrack(trackNumber, _folder, _track);
    uint16_t parameter = (_folder << 8) | _track;

    // Call base class for setting status, duration and trackName
    PlayerController::playSoundSetStatus(track, durationMs, trackName);

    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);
    Serial.printf("  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);
//    DEBUG_PRINT(DebugLevel::COMMANDS, "  ▶️ %s - track: %u (Dec)", __PRETTY_FUNCTION__, track);

    mdPlayerCommand(CMD::PLAY_FOLDER_FILE, parameter);

//    playSound(track, durationMs);
}

void MDPlayerController::stopSound() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("      %s - mdPlayerCommand(CMD::STOP_PLAY, 0)\n", __PRETTY_FUNCTION__);

    mdPlayerCommand(CMD::STOP_PLAY, 0);

    // Call base class for setting status
    PlayerController::stopSoundSetStatus();
}

void MDPlayerController::mdPlayerCommand(MDPlayerCommand command, uint16_t dat) {
    //    unsigned long startTime = millis();  // Start timing
    delay(20);
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

    for (uint8_t i = 0; i < 8; i++) {
        #if defined(ESP32)
            mySerial.write(frame[i]);
        #elif defined(ESP8266)
            mySoftwareSerial.write(frame[i]);
        #endif
//        mySoftwareSerial.write(frame[i]);
    }
    delay(20);
    //    unsigned long endTime = millis();  // End timing
    //    unsigned long duration = endTime - startTime;  // Calculate duration
    //    // Print the duration
    //    Serial.printf("mdPlayerCommand execution time: %lu ms\n", duration);
}


/**
 * @brief Sets the volume of the MD Player.
 *
 * This method sets the volume of the MD Player to the specified level. It checks if the new volume
 * is different from the last set volume to avoid unnecessary commands.
 *
 * @param playerVolume The volume level to set. Should be a value between 0 (mute) and 30 (max volume).
 *
 * @details
 * - If the new volume is different from the last set volume:
 *   - It sends a SET_VOLUME command to the MD Player with the new volume.
 *   - Updates the lastSetPlayerVolume to the new value.
 *   - If debug is enabled, it logs the volume change.
 * - If the new volume is the same as the last set volume:
 *   - It does nothing to avoid unnecessary commands.
 *   - If debug is enabled, it logs that the volume was already set to this value.
 *
 * @note This method uses the mdPlayerCommand function to send the SET_VOLUME command to the player.
 *
 * @see mdPlayerCommand
 */
 void MDPlayerController::setPlayerVolume(uint8_t playerVolume) {
    if (playerVolume != lastSetPlayerVolume) {
        mdPlayerCommand(SET_VOLUME, playerVolume);
        lastSetPlayerVolume = playerVolume;

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("%s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);

    } else {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);

    }
}

void MDPlayerController::setEqualizerPreset(EqualizerPreset preset) {
    uint8_t mdPreset;
    switch (preset) {
        case EqualizerPreset::NORMAL:
            mdPreset = 0;
            break;
        case EqualizerPreset::POP:
            mdPreset = 1;
            break;
        case EqualizerPreset::ROCK:
            mdPreset = 2;
            break;
        case EqualizerPreset::JAZZ:
            mdPreset = 3;
            break;
        case EqualizerPreset::CLASSIC:
            mdPreset = 4;
            break;
        case EqualizerPreset::BASS:
            mdPreset = 5;
            break;
        default:
            mdPreset = 0; // Default to NORMAL if an unknown preset is passed
            break;
    }

    mdPlayerCommand(SET_EQUALIZER, mdPreset);

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("MDPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));

    // call base class for status
    PlayerController::setEqualizerPreset(preset);
}

void MDPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any MD-specific update logic here if needed
}
