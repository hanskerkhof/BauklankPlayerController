#pragma once
// #pragma GCC system_header

#if __has_include("debug.h")
    #include "debug.h"
#endif

#ifndef DISPLAY_PLAYER_STATUS_ENABLED
// Display player status on a player state change.
#define DISPLAY_PLAYER_STATUS_ENABLED false
#endif

#ifndef DISPLAY_PLAYER_STATUS_PERIODIC
// Display player status every 5 seconds.
#define DISPLAY_PLAYER_STATUS_PERIODIC false
#endif

#include <stdint.h>

class PlayerController {
public:
    static const uint8_t MIN_VOLUME = 0;
    static const uint8_t MAX_VOLUME = 30;
    static const int DEFAULT_VOLUME = 15;
    static const int MIN_FADE_DURATION_MS = 1400;  // Add this line

    enum PlayerStatus {
        STATUS_STOPPED,
        STATUS_PLAYING
    };

    enum class FadeDirection {
        NONE,
        IN,
        OUT
    };

//| const             | value  |
//| :---------------- | :----- |
//| `DY::Eq::Normal`  | `0x00` |
//| `DY::Eq::Pop`     | `0x01` |
//| `DY::Eq::Rock`    | `0x02` |
//| `DY::Eq::Jazz`    | `0x03` |
//| `DY::Eq::Classic` | `0x04` |

// DFRobotDFPlayerMini.h
//#define DFPLAYER_EQ_NORMAL 0
//#define DFPLAYER_EQ_POP 1
//#define DFPLAYER_EQ_ROCK 2
//#define DFPLAYER_EQ_JAZZ 3
//#define DFPLAYER_EQ_CLASSIC 4
//#define DFPLAYER_EQ_BASS 5

// DY player
//typedef enum class Eq : byte
//  {
//    Normal,
//    Pop,
//    Rock,
//    Jazz,
//    Classic
//  } eq_t;

// EQ_NORMAL
// EQ_POP
// EQ_ROCK
// EQ_JAZZ
// EQ_CLASSIC
// EQ_BASS

    enum class EqualizerPreset : uint8_t {
        NORMAL = 0,
        POP = 1,
        ROCK = 2,
        JAZZ = 3,
        CLASSIC = 4,
        BASS = 5
    };

    // Helpers
    const char* playerStatusToString(PlayerStatus status) {
        switch (status) {
            case STATUS_STOPPED: return "STATUS_STOPPED";
            case STATUS_PLAYING: return "STATUS_PLAYING";
            default: return "UNKNOWN_STATUS";
        }
    }

    virtual const char* getPlayerTypeName() const { return "Unknown"; }

    virtual void begin() = 0;
    virtual void setVolume(int volume);
    int getVolume();

    virtual void fadeIn(int durationMs, int targetVolume, int playTrack, unsigned long trackDurationMs, const char* trackName);
    virtual void fadeOut(int durationMs, int targetVolume, bool stopSound);
    virtual void fadeTo(int durationMs,int targetVolume);
    void stopFade(bool stopSound = false);
    bool isFading();
    bool isFadingIn();
    bool isFadingOut();

    virtual void playSound(int track, unsigned long durationMs, const char* trackName);

    virtual void playSoundSetStatus(int track, unsigned long durationMs, const char* trackName);
    virtual void stopSoundSetStatus();
    virtual void stopSound();

    virtual void enableLoop() = 0;
    virtual void disableLoop() = 0;

    virtual void setEqualizerPreset(EqualizerPreset preset) = 0;
    static const char* equalizerPresetToString(EqualizerPreset preset);

    void displayPlayerStatusBox();
    void displayVolumeProgressBar();
    void displayEqualizerSettings();

//    void playSoundRandom(int minTrack, int maxTrack);
    void update();
    bool isSoundPlaying() const { return playerStatus == STATUS_PLAYING; }
    const char* createProgressBar(int value, int maxLength);


protected:
    bool isLooping = false;
//    int lastPlayedTrack = -1;  // Add this line

    virtual void setPlayerVolume(uint8_t playerVolume) = 0;

    PlayerStatus playerStatus;
//    PlayerStatus playerStatus = STATUS_STOPPED;  // Add this line
    unsigned long playStartTime = 0;
    unsigned long playDuration = 0;

    FadeDirection fadeDirection = FadeDirection::NONE;
    unsigned long lastFadeTime = 0;

    // Volume-related variables
    int currentVolume;
    int targetVolume;
    int volumeStep;

    void decodeFolderAndTrack(uint16_t trackNumber, uint8_t& folder, uint8_t& track);
    static const int DEFAULT_FADE_INTERVAL_MS = 80;
    int fadeIntervalMs = DEFAULT_FADE_INTERVAL_MS;
    bool shouldStopAfterFade = false;

private:
    const char* fadeDirectionToString(FadeDirection direction);
    unsigned long fadeStartTime;
    int currentTrack;
    const char* currentTrackName;

    EqualizerPreset currentEqualizerPreset = EqualizerPreset::NORMAL;
    int bassLevel = 50;  // Example default value
    int midLevel = 50;   // Example default value
    int trebleLevel = 50; // Example default value
};
