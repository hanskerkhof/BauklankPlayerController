//Example (play track 2):
//byte playXX[] = { 0xAA, 0x07, 0x02, 0x00, 0x02, 0xB5 };
//                │     │     │     │     │     └ checksum
//                │     │     │     │     └ L (track)
//                │     │     │     └ H (track)
//                │     │     └ data len = 2
//                │     └ command type = specifySong
//                └ start byte

//Useful commands from the sketch:
// Control
// 0x02: play, 0x03: pause, 0x04: stop, 0x14/0x15: vol± ...

// Settings
// setVol           {0xAA, 0x13, 0x01, VOL,  SM}   VOL 0..30
// setLoopMode      {0xAA, 0x18, 0x01, LM,   SM}   LM 0..7
// setEQ            {0xAA, 0x1A, 0x01, EQ,   SM}   EQ 0..4
// specifySong      {0xAA, 0x07, 0x02, H, L, SM}   track H/L

#pragma once
#include <Arduino.h>
#include "BauklankPlayerController.h"

#if defined(ESP32)
  #include <HardwareSerial.h>
#else
  #include <SoftwareSerial.h>
#endif

// ── ACK settings ─────────────────────────────────────────────────────────────
// Set to true to enable ACK-based retry for every command sent to the XY-V17B.
// The module sends 0xAA 0xFF ... after each accepted command. If no ACK arrives
// within XY_ACK_TIMEOUT_MS the command is resent up to XY_ACK_MAX_RETRIES times.
// Disable if RX is not wired or for lower latency at the cost of reliability.
#ifndef XY_ACK_ENABLED
  #define XY_ACK_ENABLED true
#endif

class XYPlayerController : public PlayerController {
public:
    // XY is pretty quick; we can be a bit tighter than DFPlayer
    static constexpr uint16_t XY_CMD_GAP_MS        = 60;   // normal gap
    static constexpr uint16_t XY_AFTER_PLAY_GAP_MS = 200;

    // ACK / retry tuning
    static constexpr uint16_t XY_ACK_TIMEOUT_MS  = 150;  // max wait for 0xAA 0xFF response
    static constexpr uint8_t  XY_ACK_MAX_RETRIES = 3;    // attempts before giving up
    static constexpr uint16_t XY_ACK_RETRY_GAP_MS = 20;  // pause between retries

#if defined(ESP32)
    XYPlayerController(int rxPin, int txPin, int uart = 2);
#else
    XYPlayerController(int rxPin, int txPin);
#endif

    void begin() override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
    void playTrack(int track, unsigned long durationMs, const char* trackName) override;
    void stop() override;
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    const char* getPlayerTypeName() const override { return "XY Player"; }
    //    void update() override;
    void update();

protected:
    void setPlayerVolume(uint8_t playerVolume) override;

    // --- Base opcode bridge for XY-V17B ---
    const char* cmdName(uint8_t type) const override {
        switch (type) {
            case XyCmd_PlayTrack: return "PlayTrack";
            case XyCmd_Stop:      return "Stop";
            case XyCmd_LoopOn:    return "LoopOn";
            case XyCmd_LoopOff:   return "LoopOff";
            case XyCmd_Volume:    return "Volume";
            case XyCmd_Eq:        return "Eq";
            default:              return "?";
        }
    }

    uint16_t normalGapMs()    const override { return XY_CMD_GAP_MS; }
    uint16_t afterPlayGapMs() const override { return XY_AFTER_PLAY_GAP_MS; }
    bool     isPlayCommand(uint8_t type) const override {
        return type == XyCmd_PlayTrack;
    }

private:
    enum : uint8_t {
        XyCmd_None = 0,
        XyCmd_PlayTrack,
        XyCmd_Stop,
        XyCmd_LoopOn,
        XyCmd_LoopOff,
        XyCmd_Volume,
        XyCmd_Eq
    };

    void sendCommand(uint8_t type, uint16_t a, uint16_t b) override;
    void sendFrame(uint8_t cmd, const uint8_t* data, uint8_t len);

    // ACK helpers — only active when XY_ACK_ENABLED == true
    bool sendFrameWithAck(uint8_t cmd, const uint8_t* data, uint8_t len);
    bool waitForAck(uint16_t timeoutMs);
    void drainRx();

#if defined(ESP32)
    HardwareSerial _serial;
#else
    SoftwareSerial _serial;
#endif

    uint8_t _lastSetPlayerVolume = 255; // invalid => force first write
    bool    _loopEnabled = false;
};
