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
  void sendCommand(uint8_t, uint16_t, uint16_t) override;

    DY::Player myDYPlayer;
    int8_t lastSetPlayerVolume = -1;
    uint8_t currentVolume;

    #if defined(ESP32)
        HardwareSerial mySerial;
    #else
        SoftwareSerial mySoftwareSerial;
    #endif

protected:
  // --- Base opcode bridge for DY player ---
  enum : uint8_t {
    DYCmd_None = 0,
    DYCmd_PlayTrack,      // a = track number (we reconstruct the /%05u.mp3 path)
    DYCmd_Stop,
    DYCmd_SetCycle,       // a = 0 (OneOff) or 1 (RepeatOne)
    DYCmd_Volume,         // a = 0..30
    DYCmd_Eq              // a = 0..5  (Normal/Pop/Rock/Jazz/Classic/Bass->Normal)
  };

  // timings for DY player (pick what feels safe)
  uint16_t normalGapMs()    const override { return 80; }   // e.g. 60–100 ms
  uint16_t afterPlayGapMs() const override { return 180; }  // after PLAY a bit longer
  bool isPlayCommand(uint8_t) const override { return false; } // (not used yet)

  const char* cmdName(uint8_t t) const override {
    switch (t) {
      case DYCmd_PlayTrack: return "PlayTrack";
      case DYCmd_Stop:      return "Stop";
      case DYCmd_SetCycle:  return "SetCycle";
      case DYCmd_Volume:    return "Volume";
      case DYCmd_Eq:        return "Eq";
      default:              return "DY?";
    }
  }

public:
    const char* getPlayerTypeName() const override { return "DY Player"; }

    #if defined(ESP32)
        DYPlayerController(int rxPin, int txPin, int uart = 2);
    #else
        DYPlayerController(int rxPin, int txPin);
    #endif

    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName);
    void playTrack(int track, unsigned long durationMs, const char* trackName);
    void stop() override;
    void setPlayerVolume(uint8_t playerVolume) override;
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();
};