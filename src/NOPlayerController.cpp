#include "NOPlayerController.h"
#include <Arduino.h>

NOPlayerController::NOPlayerController(int rxPin, int txPin) {
}

void NOPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();
}

void NOPlayerController::enableLoop() {
}

void NOPlayerController::disableLoop() {
}

void NOPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void NOPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    Serial.printf("  !! Warning: NOPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void NOPlayerController::stop() {
  PlayerController::stopSoundSetStatus();
}

void NOPlayerController::setPlayerVolume(uint8_t playerVolume) {
}

void NOPlayerController::setEqualizerPreset(EqualizerPreset preset) {
}

void NOPlayerController::update() {
}
