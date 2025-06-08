#include "MDPlayerController.h"
#include <Arduino.h>

using CMD = MDPlayerController::MDPlayerCommand;

MDPlayerController::MDPlayerController(int rxPin, int txPin)
 : mySoftwareSerial(rxPin, txPin) {
    // Initialize SoftwareSerial in the constructor
    mySoftwareSerial.begin(9600);
    delay(300);  // Give some time for the serial connection to establish

    // Select TF card in the constructor
    selectTFCard();
}

void MDPlayerController::begin() {
    Serial.println(F("  SETUP - MD_PLAYER_ENABLED && PLAYER_NO_MD_LIBRARY"));
    Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);
    mySoftwareSerial.begin(9600);
    delay(300);  // (was 1000) TODO could this be a shorter delay?
}

void MDPlayerController::selectTFCard() {
    Serial.printf("  Selecting TF card\n");
    mdPlayerCommand(CMD::SEL_DEV, DEV_TF);  // select the TF card
    delay(300);
}

void MDPlayerController::enableLoop() {
    Serial.println(F("MDPlayerController: Enabling loop"));
    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = true;
}

void MDPlayerController::disableLoop() {
    Serial.println(F("DYPlayerController: Disabling loop"));
    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = false;
}

void MDPlayerController::playSound(int track) {
  Serial.printf("  %s - track: %u (Dec)\n", __PRETTY_FUNCTION__, track);
  uint16_t trackNumber = track;
  uint8_t _folder, _track;
  decodeFolderAndTrack(trackNumber, _folder, _track);
  uint16_t parameter = (_folder << 8) | _track;
  Serial.printf("  %s - _folder: %u, _track: %u\n", __PRETTY_FUNCTION__, _folder, _track);
  Serial.printf("  %s - mdPlayerCommand(CMD_PLAY_FOLDER_FILE, 0x%X (Hex), %u (Dec))\n", __PRETTY_FUNCTION__, parameter, parameter);
  mdPlayerCommand(CMD::PLAY_FOLDER_FILE, parameter);

  // Call base class for setting status
  PlayerController::playSound(track);
}

void MDPlayerController::playSound(int track, uint32_t durationMs) {
    playSound(track);
    // Call base class for setting status and timing
    PlayerController::playSound(track, durationMs);
}

void MDPlayerController::stopSound() {
    Serial.printf("🛑 [%s] Stopping sound\n", __PRETTY_FUNCTION__);
    Serial.printf("  Current player status: %s\n", playerStatusToString(playerStatus));
    mdPlayerCommand(CMD::STOP_PLAY, 0);
    playerStatus = STATUS_STOPPED;
    Serial.printf("  Command sent: STOP_PLAY\n");
    Serial.printf("  New player status: %s\n", playerStatusToString(playerStatus));
//    Serial.println("  Sound stopped successfully");
}

void MDPlayerController::mdPlayerCommand(MDPlayerCommand command, int16_t dat) {
    delay(20);
    int8_t frame[8] = { 0 };
    frame[0] = 0x7e;                // starting byte
    frame[1] = 0xff;                // version
    frame[2] = 0x06;                // The number of bytes of the command without starting byte and ending byte
    frame[3] = static_cast<int8_t>(command);
//    frame[3] = command;             //
    frame[4] = 0x00;                // 0x00 = no feedback, 0x01 = feedback
    frame[5] = (int8_t)(dat >> 8);  // data high byte
    frame[6] = (int8_t)(dat);       // data low byte
    frame[7] = 0xef;                // ending byte

    for (uint8_t i = 0; i < 8; i++) {
        mySoftwareSerial.write(frame[i]);
    }
    delay(20);
}

void MDPlayerController::setPlayerVolume(uint8_t playerVolume) {
    if (playerVolume != lastSetPlayerVolume) {
        mdPlayerCommand(SET_VOLUME, playerVolume);
        lastSetPlayerVolume = playerVolume;
//        Serial.printf("%s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
    } else {
//         Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
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
    Serial.printf("MDPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
}

void MDPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any MD-specific update logic here if needed
}
