#include "DYPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

DYPlayerController::DYPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin), myDYPlayer(&mySoftwareSerial) {
}

void DYPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();

    DEBUG_PRINT(DebugLevel::SETUP, "%s - mySoftwareSerial.begin(%d)", __PRETTY_FUNCTION__, 9600);

    mySoftwareSerial.begin(9600);
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

void DYPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {

    // Create the path for the track
    char path[11];
    sprintf(path, "/%05d.mp3", track);

    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);

    myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);
    // Call the base class for status, duration and trackName
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void DYPlayerController::stopSound() {

  DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ⏹️ %s - myDYPlayer.stop()", __PRETTY_FUNCTION__);

  myDYPlayer.stop();

  // Call base class for status
  PlayerController::stopSoundSetStatus();
}

void DYPlayerController::setPlayerVolume(uint8_t playerVolume) {
    if (playerVolume != lastSetPlayerVolume) {
      myDYPlayer.setVolume(playerVolume);
      lastSetPlayerVolume = playerVolume;

      DEBUG_PRINT(DebugLevel::COMMANDS, "  🔊 %s - Set DY Player volume to %d", __PRETTY_FUNCTION__, playerVolume);

    } else {

      DEBUG_PRINT(DebugLevel::COMMANDS, "  🔊 %s - Volume already set to %d", __PRETTY_FUNCTION__, playerVolume);

    }
}

void DYPlayerController::enableLoop() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.println(F("DYPlayerController: Enabling loop"));

    myDYPlayer.setCycleMode(DY::PlayMode::RepeatOne);
    isLooping = true;
}

void DYPlayerController::disableLoop() {

// TODO Only print this when `DebugLevel::COMMANDS` is set
Serial.println(F("DYPlayerController: Disabling loop"));

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
    Serial.printf("!!!!! setEqualizerPreset: %d", dyPreset);
    myDYPlayer.setEq(dyPreset);

      DEBUG_PRINT(DebugLevel::COMMANDS, "  🎚️ DYPlayer: Set equalizer preset to %d", static_cast<int>(preset));

    // call base class for status
    PlayerController::setEqualizerPreset(preset);
}

void DYPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any DY-specific update logic here if needed
}