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

void NOPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
}

void NOPlayerController::stopSound() {
}

 void NOPlayerController::setPlayerVolume(uint8_t playerVolume) {
}

void NOPlayerController::setEqualizerPreset(EqualizerPreset preset) {
}

void NOPlayerController::update() {
}
