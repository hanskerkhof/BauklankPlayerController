#ifndef ESP32
// Not for this architecture – compile as empty TU
#else

#include <Arduino.h>
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

#include "AKPlayerController.h"

AKPlayerController::AKPlayerController() {
  // nothing to do; real init happens in begin()
}

void AKPlayerController::begin() {
  // Call base class begin() first
  PlayerController::begin();


  // Initialize AudioKit
  auto config = i2s.defaultConfig(TX_MODE);
  config.sd_active = true;
  i2s.begin(config);

  // Make sure no default actions are active
  // TODO enable via a method in the class...
//  i2s.audioActions().clearActions();   // <— IMPORTANT: removes addVolumeActions(), etc.

  // For visibility: print what pins the driver thinks the keys are on
//  auto pins = i2s.boardPins();
//  Serial.printf("KEY pins: K1=%d K2=%d K3=%d K4=%d K5=%d K6=%d\n",
//                pins.key1, pins.key2, pins.key3, pins.key4, pins.key5, pins.key6);


  using audio_driver::AudioDriverKey;

  i2s.addAction(AudioDriverKey::KEY_VOLUME_UP, [](bool pressed, int /*pin*/, void* ctx){
    if (!pressed) return;
    auto* self = static_cast<AKPlayerController*>(ctx);
    if (self->lastSetPlayerVolume < 30)
      self->setPlayerVolume(self->lastSetPlayerVolume + 1);
  }, this);

  i2s.addAction(AudioDriverKey::KEY_VOLUME_DOWN, [](bool pressed, int /*pin*/, void* ctx){
    if (!pressed) return;
    auto* self = static_cast<AKPlayerController*>(ctx);
    if (self->lastSetPlayerVolume > 0)
      self->setPlayerVolume(self->lastSetPlayerVolume - 1);
  }, this);

    // Set initial volume
    // TODO review this code, the implementing sketch should be responsible for setting the volume
    auto vcfg = volumeStream.defaultConfig();
    vcfg.copyFrom(config);
    // vcfg.allow_boost = true; // Uncomment to activate amplification using linear control
    volumeStream.begin(vcfg);
    // volumeStream.setVolume(0.7); // Initial volume at 70%

    // true = 1-bit mode (uses CLK=14, CMD=15, D0=2)
    if (!SD_MMC.begin("/sdcard", /*mode1bit*/ true)) {
      Serial.println("SD_MMC mount failed.");
    } else {
      Serial.println("SD_MMC mounted.");
      File root = SD_MMC.open("/");
      printSDCardIndex(root);
      root.close();
    }

    // Initialize the decoder
    decoder.begin();

    // Initialize the copier
    copier.setCheckAvailableForWrite(false);
    copier.begin(decoder, audioFile);

}

void AKPlayerController::enableLoop() {
  isLooping = true;
}

void AKPlayerController::disableLoop() {
  isLooping = false;
}

void AKPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {

  Serial.printf("  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);

  // Close any previously opened file
  // TODO Only close the file if the new file is successfully opened
  if (audioFile) {
      audioFile.close();
  }

  // Format the file path for the track
  char path[32];
  sprintf(path, "/%05d.mp3", track);

  Serial.printf("Playing track: %d, path: %s\n", track, path);

  // Open the audio file
  audioFile = SD_MMC.open(path);

  // Print audio file info
  printAudioFileInfo(path);

  if (!audioFile) {
      Serial.println("Failed to open audio file!");
      return;
  }

  Serial.println("Audio file opened successfully");

  // Reinitialize the decoder
  decoder.begin();

  // Start the copier
  copier.begin(decoder, audioFile);

  // Call the base class for housekeeping
  PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void AKPlayerController::stopSound() {
  if (audioFile) {
      audioFile.close();
  }

  PlayerController::stopSoundSetStatus();
}

void AKPlayerController::setPlayerVolume(uint8_t playerVolume) {
//  Serial.println("setPlayerVolume not implemented YET...");
  if (lastSetPlayerVolume == playerVolume) {
      return; // No change needed
  }

  // Scale 0-30 to 0.0-1.0
  float volumeLevel = playerVolume / 30.0f;

  // Clamp volume between 0.0 and 1.0
  volumeLevel = max(0.0f, min(1.0f, volumeLevel));

  Serial.printf("Setting AK player volume to %.2f\n", volumeLevel);
  volumeStream.setVolume(volumeLevel);

  lastSetPlayerVolume = playerVolume;
}

void AKPlayerController::setEqualizerPreset(EqualizerPreset preset) {
  Serial.printf("EQ preset %d selected (not implemented for AK player)\n", static_cast<int>(preset));
}

void AKPlayerController::update() {
  //...
  // TODO enable via a method in the class...
  i2s.audioActions().processActions();  // poll & dispatch button actions
  // Continues audio processing
  if (audioFile && audioFile.available()) {
      if (!copier.copy()) {
          // End of file or error
          if (isLooping) {
              // If looping, seek back to the beginning
              audioFile.seek(0);
          } else {
              // If not looping, close the file
              audioFile.close();
              PlayerController::stopSoundSetStatus();
          }
      }

  }
  PlayerController::update(); // Call the base class update method
}


void AKPlayerController::printAudioFileInfo(const char* path) {
    File file = SD_MMC.open(path);
    if (!file) {
        Serial.printf("Failed to open file for info: %s\n", path);
        return;
    }

    // Print detailed file information
    Serial.println("\n========= Audio File Information =========");
    Serial.printf("File: %s\n", path);
    Serial.printf("Size: %u bytes (%.2f KB)\n", file.size(), file.size() / 1024.0);

    // Print file position
    Serial.printf("Position: %u\n", file.position());

    // Print if file is a directory
    Serial.printf("Is Directory: %s\n", file.isDirectory() ? "Yes" : "No");

    // Print file name - extract from the path instead of using getName()
    const char* fileName = path;
    // Find the last slash to extract just the filename
    const char* lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        fileName = lastSlash + 1;  // Point to character after the slash
    }
    Serial.printf("File Name: %s\n", fileName);

    // Check if it's an MP3 file
    if (strstr(path, ".mp3") || strstr(path, ".MP3")) {
        // Basic MP3 file structure checks could be added here
        Serial.println("File Type: MP3");
    }

    Serial.println("==========================================\n");

    file.close();
}

void AKPlayerController::printSDCardIndex(File dir, String path) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            // No more files
            break;
        }

        // Get file name - Using name() method which is available in SD library
        String entryName = entry.name();
        String fullPath;

        // Construct the full path
        if (path.length() > 0) {
            fullPath = path + "/" + entryName;
        } else {
            fullPath = "/" + entryName;
        }

        // Skip hidden files and directories (starting with a dot)
        if (entryName.startsWith(".")) {
            entry.close();
            continue;
        }

        if (entry.isDirectory()) {
            Serial.print("DIR : ");
            Serial.println(fullPath);
            printSDCardIndex(entry, fullPath); // Recursive call for subdirectories
        } else {
            Serial.print("FILE: ");
            Serial.print(fullPath);
            Serial.print("\t");
            Serial.print(entry.size(), DEC);
            Serial.println(" bytes");
        }
        entry.close();
    }
}
#endif
