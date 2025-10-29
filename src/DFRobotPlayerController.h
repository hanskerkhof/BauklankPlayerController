
#pragma once
#include <Arduino.h>
#include "BauklankPlayerController.h"
#include "DFRobotDFPlayerMini.h"

#if defined(ESP32)
    #include <HardwareSerial.h>
#else
    #include <SoftwareSerial.h>
#endif

class DFRobotPlayerController : public PlayerController {
public:
    // static constexpr uint16_t DF_CMD_GAP_MS = 120;   // safe gap between commands
    static constexpr uint16_t DF_CMD_GAP_MS = 120;      // safe gap between commands

    #if defined(ESP32)
        DFRobotPlayerController(int rxPin, int txPin, int uart = 2);
    #else
        DFRobotPlayerController(int rxPin, int txPin);
    #endif

    virtual void begin() override;
    virtual void playSound(int track, unsigned long durationMs, const char* trackName);
    virtual void playTrack(int track, unsigned long durationMs, const char* trackName);
    virtual void stop() override;
    virtual void enableLoop() override;
    virtual void disableLoop() override;
    virtual void setEqualizerPreset(EqualizerPreset preset) override;
    const char* getPlayerTypeName() const override { return "DFPlayer"; }
    virtual void update();

protected:
    virtual void setPlayerVolume(uint8_t playerVolume) override;

  // --- Base opcode bridge for DFRobot player ---
  const char* cmdName(uint8_t type) const override {
    switch (type) {
      case DFCmd_PlayTrack: return "PlayTrack";
      case DFCmd_Stop:      return "Stop";
      case DFCmd_LoopOn:    return "LoopOn";
      case DFCmd_LoopOff:   return "LoopOff";
      case DFCmd_Volume:    return "Volume";
      case DFCmd_Eq:        return "Eq";
      default:              return "?";
    }
  }

    // timings for DF (slower)
    uint16_t normalGapMs()    const override { return 120; }
    uint16_t afterPlayGapMs() const override { return 250; }
    bool     isPlayCommand(uint8_t type) const override { return type == DFCmd_PlayTrack; }
    // ^^^ v2

private:
    // ---- v2
    enum : uint8_t { DFCmd_None=0, DFCmd_PlayTrack, DFCmd_Stop, DFCmd_LoopOn, DFCmd_LoopOff, DFCmd_Volume, DFCmd_Eq };
    void     sendCommand(uint8_t type, uint16_t a, uint16_t b) override;

    #if defined(ESP32)
        HardwareSerial mySerial;
    #else
        SoftwareSerial mySoftwareSerial;
    #endif

    DFRobotDFPlayerMini myDFPlayer;
    uint8_t lastSetPlayerVolume = 255; // Invalid value to force first update
};

