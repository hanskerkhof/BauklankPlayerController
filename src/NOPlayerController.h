#pragma once
#include "BauklankPlayerController.h"

class NOPlayerController : public PlayerController {
public:
    const char* getPlayerTypeName() const override { return "NO Player"; }
    NOPlayerController(int rxPin, int txPin);
    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
    void playTrack(int track, unsigned long durationMs, const char* trackName) override;
    void stop();
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();

protected:
  // timings for MD/DY player (pick what feels safe)
  uint16_t normalGapMs()    const override { return 80; }   // e.g. 60–100 ms
  uint16_t afterPlayGapMs() const override { return 180; }  // after PLAY a bit longer
  bool isPlayCommand(uint8_t) const override { return false; } // (not used yet)

  // optional (pretty debug names; not required to compile)
  const char* cmdName(uint8_t) const override { return "MD"; }

private:
    void sendCommand(uint8_t, uint16_t, uint16_t) override {}
    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;

protected:
    void setPlayerVolume(uint8_t playerVolume) override;

private:

};
