#include <Arduino.h>
#include "XYPlayerController.h"
#include "DebugLevelManager.h"

#if defined(ESP32)
XYPlayerController::XYPlayerController(int rxPin, int txPin, int uart)
    : _serial(uart) {
    _serial.begin(9600, SERIAL_8N1, rxPin, txPin);
}
#else
XYPlayerController::XYPlayerController(int rxPin, int txPin)
    : _serial(rxPin, txPin) {
    // begin() will be called in begin()
}
#endif

void XYPlayerController::begin() {
    PlayerController::begin();

#if !defined(ESP32)
    if (!_serial.isListening()) {
        _serial.begin(9600);
    }
#endif

    Serial.println(F("[XY] XY-V17B player initialized (9600 8N1)."));

    // --- Force SD card as current drive (0x0B, Drive=0x01) ---
    {
        uint8_t driveData[1] = { 0x01 };  // 01 = SD
        Serial.println(F("[XY] Switch drive to SD (0x01)"));
        sendFrameWithAck(0x0B, driveData, 1);
        delay(50);
    }

    // Optionally set some defaults: volume, EQ, loop mode
    // setPlayerVolume(DEFAULT_VOLUME);
    setEqualizerPreset(EqualizerPreset::NORMAL);
    disableLoop();  // default: single-stop
}

void XYPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
    if (track <= 0) track = 1;
    if (track > 65535) track = 65535;

    Serial.printf("  ▶️ %s - track: %u '%s', duration: %lu ms\n",
                  "XYPlayerController::playTrack",
                  track,
                  trackName ? trackName : "",
                  durationMs);

    // For XY: 'a' = 16-bit track number, H/L encoded in sendCommand()
    executePlayerCommandNowBase(XyCmd_PlayTrack, static_cast<uint16_t>(track), 0);

    // Base handles status / timers
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void XYPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    Serial.printf("  !! Warning: XYPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
    playTrack(track, durationMs, trackName);
}

void XYPlayerController::stop() {
    Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);

    executePlayerCommandNowBase(XyCmd_Stop);

    // Update base state
    PlayerController::stopSoundSetStatus();
}

void XYPlayerController::setPlayerVolume(uint8_t v) {
    v = constrain(v, MIN_VOLUME, MAX_VOLUME);
    if (v == _lastSetPlayerVolume) return;
    // Use fire-and-forget (no ACK wait) for volume to avoid blocking the fade
    // loop — each sendFrameWithAck blocks up to 150 ms, which adds 4.5 s to a
    // 30-step 3000 ms fade. Volume steps are best-effort; a missed step only
    // causes a minor non-linearity, not an audible glitch.
    uint8_t data[1] = { v };
    sendFrame(0x13, data, 1);
    _lastSetPlayerVolume = v;
}

void XYPlayerController::enableLoop() {
    _loopEnabled = true;
    executePlayerCommandBase(XyCmd_LoopOn);
}

void XYPlayerController::disableLoop() {
    _loopEnabled = false;
    executePlayerCommandBase(XyCmd_LoopOff);
}

void XYPlayerController::setEqualizerPreset(EqualizerPreset preset) {
    // XY supports NORMAL..CLASSIC (0..4); if BASS, fall back to NORMAL
    uint8_t eqCode = 0;

    switch (preset) {
        case EqualizerPreset::POP:     eqCode = 1; break;
        case EqualizerPreset::ROCK:    eqCode = 2; break;
        case EqualizerPreset::JAZZ:    eqCode = 3; break;
        case EqualizerPreset::CLASSIC: eqCode = 4; break;
        case EqualizerPreset::BASS:    // not supported on XY: map to NORMAL
        case EqualizerPreset::NORMAL:
        default:
            eqCode = 0;
            break;
    }

    executePlayerCommandNowBase(XyCmd_Eq, eqCode, 0);
    PlayerController::setEqualizerPreset(preset);
}

void XYPlayerController::sendFrame(uint8_t cmd, const uint8_t* data, uint8_t len) {
    // Build: [0xAA, cmd, len, data..., checksum]
    uint8_t buf[3 + 255 + 1];  // len max 255 -> safe
    if (len > 255) len = 255;

    buf[0] = 0xAA;
    buf[1] = cmd;
    buf[2] = len;

    uint16_t sum = buf[0] + buf[1] + buf[2];

    for (uint8_t i = 0; i < len; ++i) {
        buf[3 + i] = data ? data[i] : 0;
        sum += buf[3 + i];
    }

    buf[3 + len] = static_cast<uint8_t>(sum & 0xFF);  // low 8 bits

    // Debug: dump raw frame bytes
    Serial.print(F("[XY RAW]"));
    uint8_t totalLen = 3 + len + 1;
    for (uint8_t i = 0; i < totalLen; ++i) {
        Serial.print(' ');
        if (buf[i] < 0x10) Serial.print('0');
        Serial.print(buf[i], HEX);
    }
    Serial.println();

    // Send over UART
    _serial.write(buf, totalLen);
    _serial.flush();
}

// ── ACK helpers ───────────────────────────────────────────────────────────────

void XYPlayerController::drainRx() {
    while (_serial.available()) _serial.read();
}

bool XYPlayerController::waitForAck(uint16_t timeoutMs) {
    // The XY-V17B ACK frame starts with 0xAA 0xFF.
    // We use a minimal 2-byte FSM — we don't validate the full checksum
    // because the response length varies and we just need command acceptance.
    uint32_t deadline = millis() + timeoutMs;
    uint8_t  state    = 0;  // 0 = waiting for 0xAA, 1 = waiting for 0xFF

    while ((int32_t)(millis() - deadline) < 0) {
        if (!_serial.available()) continue;
        uint8_t b = _serial.read();
        if      (state == 0 && b == 0xAA) { state = 1; }
        else if (state == 1 && b == 0xFF) { return true; }
        else if (b == 0xAA)               { state = 1; }  // resync
        else                               { state = 0; }
    }
    return false;
}

bool XYPlayerController::sendFrameWithAck(uint8_t cmd, const uint8_t* data, uint8_t len) {
#if XY_ACK_ENABLED
    for (uint8_t attempt = 0; attempt < XY_ACK_MAX_RETRIES; attempt++) {
        drainRx();                       // clear stale RX bytes before send
        sendFrame(cmd, data, len);
        if (waitForAck(XY_ACK_TIMEOUT_MS)) {
            if (attempt > 0) {
                Serial.printf("[XY] ACK ok after %d retr%s\n",
                              attempt, attempt == 1 ? "y" : "ies");
            }
            return true;
        }
        Serial.printf("[XY] no ACK (attempt %d/%d)\n",
                      attempt + 1, XY_ACK_MAX_RETRIES);
        delay(XY_ACK_RETRY_GAP_MS);
    }
    Serial.println(F("[XY] WARNING: command not ACK'd after all retries"));
    return false;
#else
    sendFrame(cmd, data, len);
    return true;  // no ACK check, always optimistic
#endif
}

void XYPlayerController::sendCommand(uint8_t type, uint16_t a, uint16_t b) {
    // a = track/volume/eq code etc.
    uint8_t data[3] = {0};

    Serial.print(F("[WIRE:XY] "));
    switch (type) {
        case XyCmd_PlayTrack: {
            // specifySong: cmd=0x07, len=2, data=H,L of track
            uint16_t track = a;
            uint8_t H = (track >> 8) & 0xFF;
            uint8_t L = track & 0xFF;
            data[0] = H;
            data[1] = L;
            Serial.print(F("specifySong("));
            Serial.print(track);
            Serial.println(F(")"));
            sendFrameWithAck(0x07, data, 2);
            break;
        }

        case XyCmd_Stop: {
            // stop: cmd=0x04, len=0
            Serial.println(F("stop()"));
            sendFrameWithAck(0x04, nullptr, 0);
            break;
        }

        case XyCmd_Volume: {
            // setVol: cmd=0x13, len=1, data=0..30
            uint8_t vol = (uint8_t)constrain(a, MIN_VOLUME, MAX_VOLUME);
            data[0] = vol;
            Serial.print(F("setVol("));
            Serial.print(vol);
            Serial.println(F(")"));
            sendFrameWithAck(0x13, data, 1);
            _lastSetPlayerVolume = vol;
            break;
        }

        case XyCmd_LoopOn: {
            // setLoopMode: cmd=0x18, len=1, LM=01 (single cycle current song)
            // Spec: 00 all-cycle, 01 single-cycle, 02 single-stop, etc.
            data[0] = 0x01;  // Single cycle current track
            Serial.println(F("loop(single track)"));
            sendFrameWithAck(0x18, data, 1);
            break;
        }

        case XyCmd_LoopOff: {
            // Back to single-stop: LM=0x02
            data[0] = 0x02;  // Single stop
            Serial.println(F("loop disabled (single stop)"));
            sendFrameWithAck(0x18, data, 1);
            break;
        }

        case XyCmd_Eq: {
            // setEQ: cmd=0x1A, len=1, data=0..4
            uint8_t eqCode = (uint8_t)a;
            if (eqCode > 4) eqCode = 0;
            data[0] = eqCode;
            Serial.print(F("setEQ("));
            Serial.print(eqCode);
            Serial.println(F(")"));
            sendFrameWithAck(0x1A, data, 1);
            break;
        }

        default:
            Serial.println(F("UNKNOWN"));
            break;
    }
}

void XYPlayerController::update() {
    // Reuse all base logic: fades, durations, periodic status, command flush
    PlayerController::update();
}

//#include <Arduino.h>
//#include "XYPlayerController.h"
//#include "DebugLevelManager.h"
//
//#if defined(ESP32)
//XYPlayerController::XYPlayerController(int rxPin, int txPin, int uart)
//    : _serial(uart) {
//    _serial.begin(9600, SERIAL_8N1, rxPin, txPin);
//}
//#else
//XYPlayerController::XYPlayerController(int rxPin, int txPin)
//    : _serial(rxPin, txPin) {
//    // begin() will be called in begin()
//}
//#endif
//
//void XYPlayerController::begin() {
//    PlayerController::begin();
//
//#if !defined(ESP32)
//    if (!_serial.isListening()) {
//        _serial.begin(9600);
//    }
//#endif
//
//    Serial.println(F("[XY] XY-V17B player initialized (9600 8N1)."));
//
//    // Optionally set some defaults: volume, EQ, loop mode
//    setPlayerVolume(DEFAULT_VOLUME);
//    setEqualizerPreset(EqualizerPreset::NORMAL);
//    disableLoop();  // default: single-stop
//}
//
//void XYPlayerController::playTrack(int track, unsigned long durationMs, const char* trackName) {
//    if (track <= 0) track = 1;
//    if (track > 65535) track = 65535;
//
//    Serial.printf("  ▶️ %s - track: %u '%s', duration: %lu ms\n",
//                  "XYPlayerController::playTrack",
//                  track,
//                  trackName ? trackName : "",
//                  durationMs);
//
//    // For XY: 'a' = 16-bit track number, H/L encoded in sendCommand()
//    executePlayerCommandBase(XyCmd_PlayTrack, static_cast<uint16_t>(track), 0);
//
//    // Base handles status / timers
//    PlayerController::playSoundSetStatus(track, durationMs, trackName);
//}
//
//void XYPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
//    Serial.printf("  !! Warning: XYPlayerController::playSound will be deprecated in v3. Use playTrack instead.\n");
//    playTrack(track, durationMs, trackName);
//}
//
//void XYPlayerController::stop() {
//    Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
//
//    executePlayerCommandBase(XyCmd_Stop);
//
//    // Update base state
//    PlayerController::stopSoundSetStatus();
//}
//
//void XYPlayerController::setPlayerVolume(uint8_t v) {
//    v = constrain(v, MIN_VOLUME, MAX_VOLUME);
//    if (v == _lastSetPlayerVolume) return;
//    _lastSetPlayerVolume = v;
//
//    executePlayerCommandBase(XyCmd_Volume, v, 0);
//}
//
//void XYPlayerController::enableLoop() {
//    _loopEnabled = true;
//    executePlayerCommandBase(XyCmd_LoopOn);
//}
//
//void XYPlayerController::disableLoop() {
//    _loopEnabled = false;
//    executePlayerCommandBase(XyCmd_LoopOff);
//}
//
//void XYPlayerController::setEqualizerPreset(EqualizerPreset preset) {
//    // XY supports NORMAL..CLASSIC (0..4); if BASS, fall back to NORMAL
//    uint8_t eqCode = 0;
//
//    switch (preset) {
//        case EqualizerPreset::POP:     eqCode = 1; break;
//        case EqualizerPreset::ROCK:    eqCode = 2; break;
//        case EqualizerPreset::JAZZ:    eqCode = 3; break;
//        case EqualizerPreset::CLASSIC: eqCode = 4; break;
//        case EqualizerPreset::BASS:    // not supported on XY: map to NORMAL
//        case EqualizerPreset::NORMAL:
//        default:
//            eqCode = 0;
//            break;
//    }
//
//    executePlayerCommandBase(XyCmd_Eq, eqCode, 0);
//    PlayerController::setEqualizerPreset(preset);
//}
//
//void XYPlayerController::sendFrame(uint8_t cmd, const uint8_t* data, uint8_t len) {
//    // Build: [0xAA, cmd, len, data..., checksum]
//    uint8_t buf[3 + 255 + 1];  // len max 255 -> safe
//    if (len > 255) len = 255;
//
//    buf[0] = 0xAA;
//    buf[1] = cmd;
//    buf[2] = len;
//
//    uint16_t sum = buf[0] + buf[1] + buf[2];
//
//    for (uint8_t i = 0; i < len; ++i) {
//        buf[3 + i] = data ? data[i] : 0;
//        sum += buf[3 + i];
//    }
//
//    buf[3 + len] = static_cast<uint8_t>(sum & 0xFF);  // low 8 bits
//
//    // Send over UART
//#if defined(ESP32)
//    _serial.write(buf, 3 + len + 1);
//    _serial.flush();
//#else
//    _serial.write(buf, 3 + len + 1);
//    _serial.flush();
//#endif
//}
//
//void XYPlayerController::sendCommand(uint8_t type, uint16_t a, uint16_t b) {
//    // a = track/volume/eq code etc.
//    uint8_t data[3] = {0};
//
//    Serial.print(F("[WIRE:XY] "));
//    switch (type) {
//        case XyCmd_PlayTrack: {
//            // specifySong: cmd=0x07, len=2, data=H,L of track
//            uint16_t track = a;
//            uint8_t H = (track >> 8) & 0xFF;
//            uint8_t L = track & 0xFF;
//            data[0] = H;
//            data[1] = L;
//            Serial.print(F("specifySong("));
//            Serial.print(track);
//            Serial.println(F(")"));
//            sendFrame(0x07, data, 2);
//            break;
//        }
//
//        case XyCmd_Stop: {
//            // stop: cmd=0x04, len=0
//            Serial.println(F("stop()"));
//            sendFrame(0x04, nullptr, 0);
//            break;
//        }
//
//        case XyCmd_Volume: {
//            // setVol: cmd=0x13, len=1, data=0..30
//            uint8_t vol = (uint8_t)constrain(a, MIN_VOLUME, MAX_VOLUME);
//            data[0] = vol;
//            Serial.print(F("setVol("));
//            Serial.print(vol);
//            Serial.println(F(")"));
//            sendFrame(0x13, data, 1);
//            break;
//        }
//
//        case XyCmd_LoopOn: {
//            // setLoopMode: cmd=0x18, len=1, LM=01 (single cycle current song)
//            // Spec: 00 all-cycle, 01 single-cycle, 02 single-stop, etc.
//            data[0] = 0x01;  // Single cycle current track
//            Serial.println(F("loop(single track)"));
//            sendFrame(0x18, data, 1);
//            break;
//        }
//
//        case XyCmd_LoopOff: {
//            // Back to single-stop: LM=0x02
//            data[0] = 0x02;  // Single stop
//            Serial.println(F("loop disabled (single stop)"));
//            sendFrame(0x18, data, 1);
//            break;
//        }
//
//        case XyCmd_Eq: {
//            // setEQ: cmd=0x1A, len=1, data=0..4
//            uint8_t eqCode = (uint8_t)a;
//            if (eqCode > 4) eqCode = 0;
//            data[0] = eqCode;
//            Serial.print(F("setEQ("));
//            Serial.print(eqCode);
//            Serial.println(F(")"));
//            sendFrame(0x1A, data, 1);
//            break;
//        }
//
//        default:
//            Serial.println(F("UNKNOWN"));
//            break;
//    }
//}
//
//void XYPlayerController::update() {
//    // Reuse all base logic: fades, durations, periodic status, command flush
//    PlayerController::update();
//}
