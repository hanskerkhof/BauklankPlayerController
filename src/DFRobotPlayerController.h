
#pragma once
#include "BauklankPlayerController.h"
#include "DFRobotDFPlayerMini.h"

#if defined(ESP32)
    #include <HardwareSerial.h>
#else
    #include <SoftwareSerial.h>
#endif

class DFRobotPlayerController : public PlayerController {
public:
    #if defined(ESP32)
        DFRobotPlayerController(int rxPin, int txPin, int uart = 2);
    #else
        DFRobotPlayerController(int rxPin, int txPin);
    #endif

    virtual void begin() override;
    virtual void playSound(int track, unsigned long durationMs, const char* trackName);
    virtual void stopSound() override;
    virtual void enableLoop() override;
    virtual void disableLoop() override;
    virtual void setEqualizerPreset(EqualizerPreset preset) override;
    const char* getPlayerTypeName() const override { return "DFPlayer"; }
    virtual void update();

protected:
    virtual void setPlayerVolume(uint8_t playerVolume) override;

private:
    #if defined(ESP32)
        HardwareSerial mySerial;
    #else
        SoftwareSerial mySoftwareSerial;
    #endif

    DFRobotDFPlayerMini myDFPlayer;
    uint8_t lastSetPlayerVolume = 255; // Invalid value to force first update
};

//#pragma once
//
//#include "BauklankPlayerController.h"
//#if defined(ESP32)
//    #include <HardwareSerial.h>
//#else
//    #include <SoftwareSerial.h>
//#endif
////#include <SoftwareSerial.h>
//// MAKE SURE YOU ARE USING VERSION 1.0.5 OF THE DFRobotDFPlayerMini LIBRARY!!
//#include <DFRobotDFPlayerMini.h> // https://github.com/DFRobot/DFRobotDFPlayerMini
//
//class DFRobotPlayerController : public PlayerController {
//private:
//    #if defined(ESP32)
//        HardwareSerial mySerial;
//    #else
//        SoftwareSerial mySoftwareSerial;
//    #endif
////    SoftwareSerial mySoftwareSerial;
//    DFRobotDFPlayerMini myDFPlayer;
//    uint8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
//    uint8_t currentVolume; // To keep track of the current volume
//    bool isLooping = false;
//public:
//    #if defined(ESP32)
//        DFRobotPlayerController(int rxPin, int txPin, int uart = 2);  // UART 2 by default
//    #else
//        DFRobotPlayerController(int rxPin, int txPin);
//    #endif
////    DFRobotPlayerController(int rxPin, int txPin);
//
//    const char* getPlayerTypeName() const override { return "DF Player"; }
//    void begin() override;
//    void playSound(int track, unsigned long durationMs, const char* trackName) override;
////    void playSound(int track, unsigned long durationMs, const char* trackName);
//    void stopSound();
//    void enableLoop() override;
//    void disableLoop() override;
//    void setEqualizerPreset(EqualizerPreset preset) override;
//    void update();
//protected:
//    void setPlayerVolume(uint8_t playerVolume) override;
//};
