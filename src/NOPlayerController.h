#pragma once

#include "BauklankPlayerController.h"
#include <SoftwareSerial.h>


class NOPlayerController : public PlayerController {
public:
    const char* getPlayerTypeName() const override { return "MD Player"; }
    NOPlayerController(int rxPin, int txPin);
    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
    void stopSound();
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();

private:
    SoftwareSerial mySoftwareSerial;
    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;
    void selectTFCard();

    // Define DEV_TF separately as it's not part of the command enum
    static constexpr uint8_t DEV_TF = 0x02;  ///< select storage device to TF card

protected:
    void setPlayerVolume(uint8_t playerVolume) override;

private:

};
