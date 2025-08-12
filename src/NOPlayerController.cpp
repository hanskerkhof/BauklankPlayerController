#include "NOPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

NOPlayerController::NOPlayerController(int rxPin, int txPin)
 : mySoftwareSerial(rxPin, txPin) {
    // Initialize SoftwareSerial in the constructor
    mySoftwareSerial.begin(9600);
    delay(300);  // Give some time for the serial connection to establish

    // Select TF card in the constructor
    selectTFCard();
}

void NOPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);

    mySoftwareSerial.begin(9600);
    delay(300);  // (was 1000) TODO could this be a shorter delay?
}

void NOPlayerController::selectTFCard() {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("  Selecting TF card\n");
//
//    noPlayerCommand(CMD::SEL_DEV, DEV_TF);  // select the TF card
//    delay(300);
}

void NOPlayerController::enableLoop() {
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.println(F("NOPlayerController: Enabling loop"));
//
//    noPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
//    isLooping = true;
}

void NOPlayerController::disableLoop() {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.println(F("DYPlayerController: Disabling loop"));
//
//    noPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
//    isLooping = false;
}

void NOPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
//    uint16_t trackNumber = track;
//    uint8_t _folder, _track;
//    decodeFolderAndTrack(trackNumber, _folder, _track);
//    uint16_t parameter = (_folder << 8) | _track;

//    // Call base class for setting status, duration and trackName
//    PlayerController::playSoundSetStatus(track, durationMs, trackName);
//
//    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);
//    DEBUG_PRINT(DebugLevel::COMMANDS, "  ▶️ %s - track: %u (Dec)", __PRETTY_FUNCTION__, track);

//    noPlayerCommand(CMD::PLAY_FOLDER_FILE, parameter);

//    playSound(track, durationMs);
}

void NOPlayerController::stopSound() {
//
//  // TODO Only print this when `DebugLevel::COMMANDS` is set
//  Serial.printf("      %s - noPlayerCommand(CMD::STOP_PLAY, 0)\n", __PRETTY_FUNCTION__);
//
//    noPlayerCommand(CMD::STOP_PLAY, 0);
//
//    // Call base class for setting status
//    PlayerController::stopSoundSetStatus();
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
 * @note This method uses the noPlayerCommand function to send the SET_VOLUME command to the player.
 *
 * @see noPlayerCommand
 */
 void NOPlayerController::setPlayerVolume(uint8_t playerVolume) {
//    if (playerVolume != lastSetPlayerVolume) {
//        noPlayerCommand(SET_VOLUME, playerVolume);
//        lastSetPlayerVolume = playerVolume;
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("%s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
//
//    } else {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
//
//    }
}

void NOPlayerController::setEqualizerPreset(EqualizerPreset preset) {
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
//    noPlayerCommand(SET_EQUALIZER, mdPreset);

//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("NOPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
//
//    // call base class for status
//    PlayerController::setEqualizerPreset(preset);
}

void NOPlayerController::update() {
//    PlayerController::update(); // Call the base class update method
//    // Add any MD-specific update logic here if needed
}
