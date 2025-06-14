// DYPlayerController.h
#ifndef DY_PLAYER_CONTROLLER_H
#define DY_PLAYER_CONTROLLER_H

#include "BauklankPlayerController.h"
#include <SoftwareSerial.h>
// Include the library
#include <DYPlayerArduino.h> // https://github.com/SnijderC/dyplayer

class DYPlayerController : public PlayerController {
private:
//    SoftwareSerial mySoftwareSerial;
    DY::Player myDYPlayer;
    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;
public:
    const char* getPlayerTypeName() const override { return "DY Player"; }
    SoftwareSerial mySoftwareSerial;
    DYPlayerController(int rxPin, int txPin);
    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
//    void playSound(int track, unsigned long durationMs, const char* trackName);
    void stopSound();
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();
protected:
    void setPlayerVolume(uint8_t playerVolume) override;
};

#endif // DY_PLAYER_CONTROLLER_H