// MDPlayerController.h
#ifndef MD_PLAYER_CONTROLLER_H
#define MD_PLAYER_CONTROLLER_H

#include "BauklankPlayerController.h"
#include <SoftwareSerial.h>


class MDPlayerController : public PlayerController {
public:
    const char* getPlayerTypeName() const override { return "MD Player"; }
    MDPlayerController(int rxPin, int txPin);
    void begin() override;
    void playSound(int track) override;
    void playSound(int track, unsigned long durationMs) override;
    void playSound(int track, unsigned long durationMs, const char* trackName) override;
    void stopSound() override;
    void enableLoop() override;
    void disableLoop() override;
    void setEqualizerPreset(EqualizerPreset preset) override;
    void update();
    using PlayerController::playSoundRandom;

    // Command definitions
    enum MDPlayerCommand : uint8_t {
        NUL = 0x00,               ///< No command
        NEXT_SONG = 0x01,         ///< Play next song
        PREV_SONG = 0x02,         ///< Play previous song
        PLAY_WITH_INDEX = 0x03,   ///< Play song with index number
        VOLUME_UP = 0x04,         ///< Volume increase by one
        VOLUME_DOWN = 0x05,       ///< Volume decrease by one
        SET_VOLUME = 0x06,        ///< Set the volume to level specified
        SET_EQUALIZER = 0x07,     ///< Set the equalizer to specified level
        SNG_CYCL_PLAY = 0x08,     ///< Loop play (repeat) specified track
        SEL_DEV = 0x09,           ///< Select storage device to TF card
        SLEEP_MODE = 0x0a,        ///< Chip enters sleep mode
        WAKE_UP = 0x0b,           ///< Chip wakes up from sleep mode
        RESET = 0x0c,             ///< Chip reset
        PLAY = 0x0d,              ///< Playback restart
        PAUSE = 0x0e,             ///< Playback is paused
        PLAY_FOLDER_FILE = 0x0f,  ///< Play the song with the specified folder and index number
        STOP_PLAY = 0x16,         ///< Playback is stopped
        FOLDER_CYCLE = 0x17,      ///< Loop playback from specified folder
        SHUFFLE_PLAY = 0x18,      ///< Playback shuffle mode
        SET_SNGL_CYCL = 0x19,     ///< Set loop play (repeat) on/off for current file
        SET_DAC = 0x1a,           ///< DAC on/off control
        PLAY_W_VOL = 0x22,        ///< Play track at the specified volume
        SHUFFLE_FOLDER = 0x28,    ///< Playback shuffle mode for folder specified
        QUERY_STATUS = 0x42,      ///< Query Device Status
        QUERY_VOLUME = 0x43,      ///< Query Volume level
        QUERY_EQUALIZER = 0x44,   ///< Query current equalizer (disabled in hardware)
        QUERY_TOT_FILES = 0x48,   ///< Query total files in all folders
        QUERY_PLAYING = 0x4c,     ///< Query which track playing
        QUERY_FLDR_FILES = 0x4e,  ///< Query total files in folder
        QUERY_TOT_FLDR = 0x4f,    ///< Query number of folders
    };
private:
    SoftwareSerial mySoftwareSerial;
    int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
    uint8_t currentVolume; // To keep track of the current volume
    bool isLooping = false;
    void mdPlayerCommand(MDPlayerCommand command, uint16_t dat);
    void selectTFCard();

    // Define DEV_TF separately as it's not part of the command enum
    static constexpr uint8_t DEV_TF = 0x02;  ///< select storage device to TF card

protected:
    void setPlayerVolume(uint8_t playerVolume) override;

private:

};

#endif // MD_PLAYER_CONTROLLER_H