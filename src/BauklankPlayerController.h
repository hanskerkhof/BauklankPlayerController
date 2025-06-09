// BauklankPlayerController.h
#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <stdint.h>

class PlayerController {
public:
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
//typedef enum class Eq : uint8_t
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
// TODO create enum for equalizer presets

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

    virtual void begin() = 0;  // Add this line
    virtual void setVolume(int volume);
    int getVolume();

    virtual void fadeIn(int durationMs, int targetVolume = 30);
    virtual void fadeIn(int durationMs, int targetVolume, int playTrack);
    virtual void fadeIn(int durationMs, int targetVolume, int playTrack, uint32_t trackDurationMs);

    virtual void fadeOut(int durationMs, int targetVolume = 0);
    virtual void fadeOut(int durationMs, int targetVolume, bool stopSound);
    virtual void fade(int durationMs, int minVolume, int maxVolume);

    virtual void playSound(int track);
    virtual void playSound(int track, uint32_t durationMs);

    virtual void enableLoop() = 0;
    virtual void disableLoop() = 0;

    virtual void setEqualizerPreset(EqualizerPreset preset) = 0;

    void playSoundRandom(int minTrack, int maxTrack);
    void update();
    bool isSoundPlaying() const { return playerStatus == STATUS_PLAYING; }
    virtual void stopSound() = 0;

protected:
    bool isLooping = false;
//    int lastPlayedTrack = -1;  // Add this line

    virtual void setPlayerVolume(uint8_t playerVolume) = 0;

    PlayerStatus playerStatus;
//    PlayerStatus playerStatus = STATUS_STOPPED;  // Add this line
    uint32_t playStartTime = 0;
    uint32_t playDuration = 0;

    FadeDirection fadeDirection = FadeDirection::NONE;
    unsigned long lastFadeTime = 0;

    // Volume-related variables
    int currentVolume;
    int targetVolume;
    int volumeStep;

    void decodeFolderAndTrack(uint16_t trackNumber, uint8_t& folder, uint8_t& track);
    static const uint8_t MIN_VOLUME = 0;
    static const uint8_t MAX_VOLUME = 30;
    static const int DEFAULT_FADE_INTERVAL_MS = 80;
    int fadeIntervalMs = DEFAULT_FADE_INTERVAL_MS;
    bool shouldStopAfterFade = false;

private:
    unsigned long fadeStartTime;
    int currentTrack;
};

#endif // PLAYER_CONTROLLER_H
