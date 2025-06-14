// DFRobotPlayerController.h
#ifndef DFROBOT_PLAYER_CONTROLLER_H
#define DFROBOT_PLAYER_CONTROLLER_H

#include "BauklankPlayerController.h"
#include <SoftwareSerial.h>
// MAKE SURE YOU ARE USING VERSION 1.0.5 OF THE DFRobotDFPlayerMini LIBRARY!!
#include <DFRobotDFPlayerMini.h> // https://github.com/DFRobot/DFRobotDFPlayerMini

class DFRobotPlayerController : public PlayerController {
private:
    SoftwareSerial mySoftwareSerial;
    DFRobotDFPlayerMini myDFPlayer;
    uint8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;
public:
    DFRobotPlayerController(int rxPin, int txPin);

    const char* getPlayerTypeName() const override { return "DF Player"; }
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

#endif // DFROBOT_PLAYER_CONTROLLER_H
