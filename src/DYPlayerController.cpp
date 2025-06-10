#include "DYPlayerController.h"
#include <Arduino.h>

DYPlayerController::DYPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin), myDYPlayer(&mySoftwareSerial) {
    // Constructor body
}

void DYPlayerController::begin() {
    Serial.println(F("  SETUP - DY_PLAYER_ENABLED"));
    Serial.printf("  SETUP - mySoftwareSerial.begin(%d)\n", 9600);
    mySoftwareSerial.begin(9600);
    delay(300);  // Give some time for the serial connection to establish
    int slp = 30;
    delay(slp);
    Serial.printf("  SETUP - DY_PLAYER_ENABLED myDYPlayer.begin()\n");
    myDYPlayer.begin();
    delay(slp);
    Serial.printf("  SETUP - DY_PLAYER_ENABLED myDYPlayer.setPlayingDevice(%d)\n", DY::Device::Sd);
    myDYPlayer.setPlayingDevice(DY::Device::Sd);
    delay(slp);
    Serial.printf("  SETUP - DY_PLAYER_ENABLED myDYPlayer.setCycleMode(%d)\n", DY::PlayMode::OneOff);
    myDYPlayer.setCycleMode(DY::PlayMode::OneOff);
    delay(slp);
    Serial.printf("  SETUP - DY_PLAYER_ENABLED myDYPlayer.setEq(%d)\n", DY::Eq::Normal);
    myDYPlayer.setEq(DY::Eq::Normal);
    delay(slp);
}

void DYPlayerController::playSound(int track) {
    // Call base class first for setting status
    PlayerController::playSound(track);

    Serial.printf("  %s - track: %u (Dec)\n", __PRETTY_FUNCTION__, track);
    char path[11];
    sprintf(path, "/%05d.mp3", track);
    Serial.printf("      Playing track: %d, path: %s\n", track, path);
    Serial.printf("      myDYPlayer.playSpecifiedDevicePath(%s)\n", path);
    myDYPlayer.playSpecifiedDevicePath(DY::Device::Sd, path);

}

void DYPlayerController::playSound(int track, uint32_t durationMs) {
    // Call base class first for setting status and timing
    PlayerController::playSound(track, durationMs);

    playSound(track);
}

void DYPlayerController::playSound(int track, uint32_t durationMs, const char* trackName) {
    // Call the base class implementation
    PlayerController::playSound(track, durationMs, trackName);

    // Then call playsound with the 2 parameters
    playSound(track, durationMs);
}

void DYPlayerController::stopSound() {
    myDYPlayer.stop();
    playerStatus = STATUS_STOPPED;
}

void DYPlayerController::setPlayerVolume(uint8_t playerVolume) {
  if (playerVolume != lastSetPlayerVolume) {
      myDYPlayer.setVolume(playerVolume);
      lastSetPlayerVolume = playerVolume;
//      Serial.printf("%s - Set DY Player volume to %d\n", __PRETTY_FUNCTION__, playerVolume);
  } else {
//       Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, playerVolume);
  }
}

void DYPlayerController::enableLoop() {
    Serial.println(F("DYPlayerController: Enabling loop"));
    myDYPlayer.setCycleMode(DY::PlayMode::RepeatOne);
    isLooping = true;
}

void DYPlayerController::disableLoop() {
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
    myDYPlayer.setEq(dyPreset);
    Serial.printf("DYPlayer: Set equalizer preset to %d\n", static_cast<int>(preset));
}

void DYPlayerController::update() {
    PlayerController::update(); // Call the base class update method
    // Add any DY-specific update logic here if needed
}