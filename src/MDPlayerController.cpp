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
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("  SETUP - MD_PLAYER_ENABLED && PLAYER_NO_MD_LIBRARY"));
        Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);
    #endif
    mySoftwareSerial.begin(9600);
    delay(300);  // (was 1000) TODO could this be a shorter delay?
}

void MDPlayerController::selectTFCard() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  Selecting TF card\n");
    #endif
    mdPlayerCommand(CMD::SEL_DEV, DEV_TF);  // select the TF card
    delay(300);
}

void MDPlayerController::enableLoop() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("MDPlayerController: Enabling loop"));
    #endif
    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = true;
}

void MDPlayerController::disableLoop() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("DYPlayerController: Disabling loop"));
    #endif
    mdPlayerCommand(CMD::SET_SNGL_CYCL, 0);  // select the TF card
    isLooping = false;
}

void MDPlayerController::playSound(int track) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - track: %u (Dec)\n", __PRETTY_FUNCTION__, track);
    #endif
    uint16_t trackNumber = track;
    uint8_t _folder, _track;
    decodeFolderAndTrack(trackNumber, _folder, _track);
    uint16_t parameter = (_folder << 8) | _track;
    mdPlayerCommand(CMD::PLAY_FOLDER_FILE, parameter);
    // Call base class for setting status
    PlayerController::playSound(track);
}

void MDPlayerController::playSound(int track, unsigned long durationMs) {
    playSound(track);

    // Call base class for setting status and duration
    PlayerController::playSound(track, durationMs);
}

void MDPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    playSound(track, durationMs);
    // Call base class for setting status, duration and trackName
    PlayerController::playSound(track, durationMs, trackName);
}

void MDPlayerController::stopSound() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
    #endif
    mdPlayerCommand(CMD::STOP_PLAY, 0);

    // Call base class for setting status
    PlayerController::stopSound();
}

void MDPlayerController::mdPlayerCommand(MDPlayerCommand command, uint16_t dat) {
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
        mySoftwareSerial.write(frame[i]);
    }
    delay(20);
}

void MDPlayerController::setPlayerVolume(uint8_t playerVolume) {
    if (playerVolume != lastSetPlayerVolume) {
        mdPlayerCommand(SET_VOLUME, playerVolume);
        lastSetPlayerVolume = playerVolume;
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
//        Serial.printf("%s - Set MD Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
        #endif
    } else {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
//         Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
        #endif
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
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("MDPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
    #endif
    // call base class for status
    PlayerController::setEqualizerPreset(preset);
}

void MDPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any MD-specific update logic here if needed
}
