#include "DYPlayerController.h"
#include <Arduino.h>

DYPlayerController::DYPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin), myDYPlayer(&mySoftwareSerial) {
}

void DYPlayerController::begin() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  %s - SETUP - DY_PLAYER_ENABLED", __PRETTY_FUNCTION__);
        Serial.printf("  %s - SETUP - mySoftwareSerial.begin(%d)\n", __PRETTY_FUNCTION__, 9600);
    #endif
    mySoftwareSerial.begin(9600);
    delay(300);  // Give some time for the serial connection to establish
    int slp = 30;
    delay(slp);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  %s - SETUP - DY_PLAYER_ENABLED myDYPlayer.begin()\n", __PRETTY_FUNCTION__);
    #endif
    myDYPlayer.begin();
    delay(slp);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  %s - SETUP - DY_PLAYER_ENABLED myDYPlayer.setPlayingDevice(%d)\n", __PRETTY_FUNCTION__, DY::Device::Sd);
    #endif
    myDYPlayer.setPlayingDevice(DY::Device::Sd);
    delay(slp);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  %s - SETUP - DY_PLAYER_ENABLED myDYPlayer.setCycleMode(%d)\n", __PRETTY_FUNCTION__, DY::PlayMode::OneOff);
    #endif
    myDYPlayer.setCycleMode(DY::PlayMode::OneOff);
    delay(slp);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  %s - SETUP - DY_PLAYER_ENABLED myDYPlayer.setEq(%d)\n", __PRETTY_FUNCTION__, DY::Eq::Normal);
    #endif
    myDYPlayer.setEq(DY::Eq::Normal);
    delay(slp);
}

void DYPlayerController::playSound(int track) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("▶️ %s - track: %u (Dec)\n", __PRETTY_FUNCTION__, track);
    #endif
    char path[11];
    sprintf(path, "/%05d.mp3", track);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
       Serial.printf("     Playing track: %d, path: %s\n", track, path);
        Serial.printf("     myDYPlayer.playSpecifiedDevicePath(%s)\n", path);
    #endif
    myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);

    // Call base class for status
    PlayerController::playSound(track);
}

void DYPlayerController::playSound(int track, unsigned long durationMs) {
    // Call base class for status and duration
    PlayerController::playSound(track, durationMs);

    playSound(track);
}

void DYPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    // Call the base class for status, duration and trackName
    PlayerController::playSound(track, durationMs, trackName);

    // Then call playsound with the 2 parameters
    // playSound(track, durationMs);
    playSound(track);
}

void DYPlayerController::stopSound() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
    #endif
    myDYPlayer.stop();

    // Call base class for status
    PlayerController::stopSound();
}

void DYPlayerController::setPlayerVolume(uint8_t playerVolume) {
    if (playerVolume != lastSetPlayerVolume) {
        myDYPlayer.setVolume(playerVolume);
        lastSetPlayerVolume = playerVolume;
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        //      Serial.printf("%s - Set DY Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
        #endif
    } else {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        //       Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
        #endif
    }
}

void DYPlayerController::enableLoop() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("DYPlayerController: Enabling loop"));
    #endif
    myDYPlayer.setCycleMode(DY::PlayMode::RepeatOne);
    isLooping = true;
}

void DYPlayerController::disableLoop() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("DYPlayerController: Disabling loop"));
    #endif
    myDYPlayer.setCycleMode(DY::PlayMode::OneOff);
    isLooping = false;
}

void DYPlayerController::setEqualizerPreset(EqualizerPreset preset) {
    DY::Eq dyPreset;
    switch (preset) {
        case EqualizerPreset::NORMAL:
            dyPreset = DY::Eq::Normal;
            break;
        case EqualizerPreset::POP:
            dyPreset = DY::Eq::Pop;
            break;
        case EqualizerPreset::ROCK:
            dyPreset = DY::Eq::Rock;
            break;
        case EqualizerPreset::JAZZ:
            dyPreset = DY::Eq::Jazz;
            break;
        case EqualizerPreset::CLASSIC:
            dyPreset = DY::Eq::Classic;
            break;
        case EqualizerPreset::BASS:
            // DY player doesn't have a BASS preset, so we'll use Normal
            dyPreset = DY::Eq::Normal;
            break;
    }
    myDYPlayer.setEq(dyPreset);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("DYPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
    #endif
    // call base class for status
    PlayerController::setEqualizerPreset(preset);
}

void DYPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any DY-specific update logic here if needed
}