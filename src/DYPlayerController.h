#pragma once

#include "BauklankPlayerController.h"

#if defined(ESP32)
    #include <HardwareSerial.h>
#else
    #include <SoftwareSerial.h>
#endif

#include <DYPlayerArduino.h>

class DYPlayerController : public PlayerController {
private:
    DY::Player myDYPlayer;
    int8_t lastSetPlayerVolume = -1;
    uint8_t currentVolume;

    #if defined(ESP32)
        HardwareSerial mySerial;
    #else
        SoftwareSerial mySoftwareSerial;
    #endif

public:
    const char* getPlayerTypeName() const override { return "DY Player"; }

    #if defined(ESP32)
        DYPlayerController(int rxPin, int txPin, int uart = 2);
    #else
        DYPlayerController(int rxPin, int txPin);
    #endif

    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName);
    void stopSound() override;
    void setPlayerVolume(uint8_t playerVolume) override;
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();
};

//#pragma once
//#include "BauklankPlayerController.h"
//
//#if defined(ESP32)
//    #include <HardwareSerial.h>
//#else
//    #include <SoftwareSerial.h>
//#endif
////#include <SoftwareSerial.h>
//
//// Include the library
//#include <DYPlayerArduino.h> // https://github.com/SnijderC/dyplayer
//
//class DYPlayerController : public PlayerController {
//private:
////    SoftwareSerial mySoftwareSerial;
//    DY::Player myDYPlayer;
//    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
//    uint8_t currentVolume; // To keep track of the current volume
//    bool isLooping = false;
//public:
//    const char* getPlayerTypeName() const override { return "DY Player"; }
//
//    #if defined(ESP32)
//        HardwareSerial mySerial;
//    #else
//        SoftwareSerial mySoftwareSerial;
//    #endif
////    SoftwareSerial mySoftwareSerial;
//
//    #if defined(ESP32)
//        DYPlayerController(int rxPin, int txPin, int uart = 2);
//    #else
//        DYPlayerController(int rxPin, int txPin);
//    #endif
//    void begin() override;
//    void playSound(int track, unsigned long durationMs, const char* trackName) override;
//    //    void playSound(int track, unsigned long durationMs, const char* trackName);
//    void stopSound();
//    void enableLoop() override;
//    void disableLoop() override;
//    void setEqualizerPreset(EqualizerPreset preset) override;
//    void update();
//protected:
//    void setPlayerVolume(uint8_t playerVolume) override;
//};
