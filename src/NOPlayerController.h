#pragma once
#include "BauklankPlayerController.h"

class NOPlayerController : public PlayerController {
public:
    const char* getPlayerTypeName() const override { return "NO Player"; }
    NOPlayerController(int rxPin, int txPin);
    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
    void stopSound();
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();

private:
    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;

protected:
    void setPlayerVolume(uint8_t playerVolume) override;

private:

};
