// AKPlayerController.h
#pragma once

#include <SPI.h>
#include <SD.h>
#include "SD_MMC.h"

// Install:
// - https://github.com/pschatzmann/arduino-audio-tools
// - https://github.com/pschatzmann/arduino-audio-driver
// - https://github.com/pschatzmann/arduino-libhelix
#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "AudioTools/Disk/AudioSourceSD.h" // or AudioSourceIdxSD.h
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

#include "BauklankPlayerController.h"

class AKPlayerController : public PlayerController {
public:
  const char* getPlayerTypeName() const override { return "AK Player"; }
  AKPlayerController();
  //  AKPlayerController(int rxPin, int txPin);
  void begin() override;
  void playSound(int track, unsigned long durationMs, const char* trackName) override;
  void playTrack(int track, unsigned long durationMs, const char* trackName) override;
  void stop();
  void enableLoop() override;
  void disableLoop() override;
  void setEqualizerPreset(EqualizerPreset preset) override;
  void update();

  // Print audio file info
  void printAudioFileInfo(const char* path);
  void printSDCardIndex(File dir, String path = "");

protected:
  void setPlayerVolume(uint8_t playerVolume) override;

  // --- Base opcode bridge for AK ---
  enum : uint8_t {
    AKCmd_None = 0,
    AKCmd_PlayTrack,   // a = track number (/%05d.mp3)
    AKCmd_Stop,
    AKCmd_SetCycle,    // a = 0 OneOff, 1 RepeatOne
    AKCmd_Volume       // a = 0..30
  };

  // AK is local + fast; small, even gaps are fine 30-60ms
  uint16_t normalGapMs()    const override { return 30; }
  uint16_t afterPlayGapMs() const override { return 60; }
  bool isPlayCommand(uint8_t t) const override { return t == AKCmd_PlayTrack; }

  const char* cmdName(uint8_t t) const override {
    switch (t) {
      case AKCmd_PlayTrack: return "PlayTrack";
      case AKCmd_Stop:      return "Stop";
      case AKCmd_SetCycle:  return "SetCycle";
      case AKCmd_Volume:    return "Volume";
      default:              return "AK?";
    }
  }

private:
  void sendCommand(uint8_t type, uint16_t a, uint16_t b) override;

  int8_t lastSetPlayerVolume = -1;  // Initialize to an invalid value
  uint8_t currentVolume; // To keep track of the current volume
  bool isLooping = false;

  // Add AudioTools components
  // Define the chip select pin for the SD card
  const int chipSelect = PIN_AUDIO_KIT_SD_CARD_CS;
  // Create an AudioBoardStream object for the final output
  AudioBoardStream i2s = AudioBoardStream(AudioKitEs8388V1); // final output of decoded stream

  // Create a Volume stream for manipulating the volume
  VolumeStream volumeStream = VolumeStream(i2s);

  // Create an EncodedAudioStream object for MP3 decoding
  EncodedAudioStream decoder = EncodedAudioStream(&volumeStream, new MP3DecoderHelix()); // Decoding stream

  // Create a StreamCopy object for copying data between streams
  StreamCopy copier;

  // Declare a File object to handle the MP3 file
  File audioFile;
};
