// PlayerController.cpp
#include "BauklankPlayerController.h"
#include <Arduino.h>

unsigned long lastPeriodicUpdate = 0;
unsigned long DISPLAY_STATUS_UPDATE_INTERVAL = 5000;

void PlayerController::setVolume(int _volume) {
    currentVolume = constrain(_volume, MIN_VOLUME, MAX_VOLUME);
    // Serial.printf("%s - Setting volume to %d\n", __PRETTY_FUNCTION__, currentVolume);
    setPlayerVolume(currentVolume);
}

int PlayerController::getVolume() {
    return currentVolume;
}

void PlayerController::playSoundRandom(int minTrack, int maxTrack) {
    int rnd = random(minTrack, maxTrack + 1);
    Serial.printf("%s - Playing random track: %d\n", __PRETTY_FUNCTION__, rnd);
    playSound(rnd);
}


void PlayerController::decodeFolderAndTrack(uint16_t trackNumber, uint8_t& folder, uint8_t& track) {
    folder = (trackNumber - 1) / 255 + 1;
    track = (trackNumber - 1) % 255 + 1;
}

void PlayerController::playSound(int track) {
    playerStatus = STATUS_PLAYING;
    currentTrack = track;
    Serial.printf("%s - Playing track: %d, playerStatus: %s\n", __PRETTY_FUNCTION__, track, playerStatusToString(playerStatus));
}

void PlayerController::playSound(int track, unsigned long durationMs) {

    // Add duration
    playStartTime = millis();
    playDuration = durationMs;

    Serial.printf("%s - Playing track: %d, duration: %d ms, startTime: %lu, endTime: %lu\n",
                  __PRETTY_FUNCTION__, track, durationMs, playStartTime, playStartTime + playDuration);

    playSound(track);
    PlayerController::displayPlayerStatusBox();
}

void PlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    // Add duration
    playStartTime = millis();
    playDuration = durationMs;
    currentTrackName = trackName;

    Serial.printf("%s - Playing track: %d (%s), duration: %d ms, startTime: %lu, endTime: %lu\n",
                  __PRETTY_FUNCTION__, track, currentTrackName, durationMs, playStartTime, playStartTime + playDuration);

    // Force statistics to print in update() loop
    lastPeriodicUpdate = millis() + 10000000;
    playSound(track);
}

void PlayerController::stopSound() {
    // TODO create a method to reset the track when it is stopped
    playerStatus = STATUS_STOPPED;
    currentTrack = 0;
    currentTrackName = "";
    Serial.printf("%s - Stop sound, playerStatus: %d\n", __PRETTY_FUNCTION__, playerStatus);
//    Serial.printf("%s - Last played track: %d, duration: %d ms, startTime: %lu, endTime: %lu\n",
//                  __PRETTY_FUNCTION__, track, playDuration, playStartTime, playStartTime + playDuration);
}

void PlayerController::setEqualizerPreset(EqualizerPreset preset) {
    // Default implementation
    Serial.print("Setting equalizer preset: ");
    switch (preset) {
        case EqualizerPreset::NORMAL:
            Serial.println("Normal");
            break;
        case EqualizerPreset::POP:
            Serial.println("Pop");
            break;
        case EqualizerPreset::ROCK:
            Serial.println("Rock");
            break;
        case EqualizerPreset::JAZZ:
            Serial.println("Jazz");
            break;
        case EqualizerPreset::CLASSIC:
            Serial.println("Classic");
            break;
        case EqualizerPreset::BASS:
            Serial.println("Bass");
            break;
        default:
            Serial.println("Unknown");
            break;
    }
    // Note: This default implementation doesn't actually change any settings.
    // Derived classes should override this method to implement actual functionality.
}

void PlayerController::fadeIn(int durationMs, int targetVolume) {
    Serial.printf("%s - Fade in requested: duration %d ms, target volume %d\n", __PRETTY_FUNCTION__, durationMs, targetVolume);

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        fadeDirection = FadeDirection::IN;
        currentVolume = constrain(getVolume(), MIN_VOLUME, MAX_VOLUME);
        this->targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);

        Serial.printf("%s - Current volume: %d, Target volume: %d\n", __PRETTY_FUNCTION__, currentVolume, this->targetVolume);

        if (currentVolume >= this->targetVolume) {
            fadeDirection = FadeDirection::NONE;
            Serial.printf("%s - No need to fade in, current volume (%d) is already at or above target (%d)\n",
                          __PRETTY_FUNCTION__, currentVolume, this->targetVolume);
            return;
        }

        int volumeDifference = this->targetVolume - currentVolume;
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        Serial.printf("%s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                      __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
    } else {
        Serial.printf("%s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
    }
}

void PlayerController::fadeIn(int durationMs, int targetVolume, int playTrack) {
    Serial.printf("%s - Fade in with track requested: duration %d ms, target volume %d, track %d\n",
                  __PRETTY_FUNCTION__, durationMs, targetVolume, playTrack);

    // Set volume to 0 before starting playback
    setVolume(0);
    currentVolume = 0;

    // Start playing the track
    playSound(playTrack);

    // Now call the regular fadeIn method
    fadeIn(durationMs, targetVolume);
}

void PlayerController::fadeIn(int durationMs, int targetVolume, int playTrack, unsigned long trackDurationMs) {
    Serial.printf("%s - Fade in with track and duration requested: fade duration %d ms, target volume %d, track %d, track duration %lu ms\n",
                  __PRETTY_FUNCTION__, durationMs, targetVolume, playTrack, trackDurationMs);

    // Set volume to 0 before starting playback
    setVolume(0);
    currentVolume = 0;

    // Start playing the track with the specified duration
    playSound(playTrack, trackDurationMs);

    // Now call the regular fadeIn method
    fadeIn(durationMs, targetVolume);
}

void PlayerController::fadeOut(int durationMs, int targetVolume, bool stopSound) {
    fadeOut(durationMs, targetVolume);
    shouldStopAfterFade = stopSound;
}

void PlayerController::fadeOut(int durationMs, int targetVolume) {
    Serial.printf("%s - Fade out requested: duration %d ms, target volume %d\n", __PRETTY_FUNCTION__, durationMs, targetVolume);

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        fadeDirection = FadeDirection::OUT;
        currentVolume = constrain(getVolume(), MIN_VOLUME, MAX_VOLUME);
        this->targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);

        Serial.printf("%s - Current volume: %d, Target volume: %d\n", __PRETTY_FUNCTION__, currentVolume, targetVolume);

        if (currentVolume <= targetVolume) {
            fadeDirection = FadeDirection::NONE;
            Serial.printf("%s - No need to fade out, current volume (%d) is already at or below target (%d)\n",
                          __PRETTY_FUNCTION__, currentVolume, targetVolume);
            return;
        }

        int volumeDifference = currentVolume - targetVolume;
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        Serial.printf("%s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                      __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
    } else {
        Serial.printf("%s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
    }
}

void PlayerController::fade(int durationMs, int minVolume, int maxVolume) {
    Serial.printf("%s - Fade requested: duration %d ms, min volume %d, max volume %d\n",
                  __PRETTY_FUNCTION__, durationMs, minVolume, maxVolume);

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        currentVolume = getVolume();

        // Constrain min and max volumes
        minVolume = constrain(minVolume, MIN_VOLUME, MAX_VOLUME);
        maxVolume = constrain(maxVolume, MIN_VOLUME, MAX_VOLUME);

        // Ensure minVolume is not greater than maxVolume
        if (minVolume > maxVolume) {
            std::swap(minVolume, maxVolume);
        }

        // Determine fade direction and target volume
        if (currentVolume < minVolume) {
            fadeDirection = FadeDirection::IN;
            targetVolume = minVolume;
        } else if (currentVolume > maxVolume) {
            fadeDirection = FadeDirection::OUT;
            targetVolume = maxVolume;
        } else {
            // Current volume is already within the specified range
            fadeDirection = FadeDirection::NONE;
            Serial.printf("%s - No need to fade, current volume (%d) is already within range (%d-%d)\n",
                          __PRETTY_FUNCTION__, currentVolume, minVolume, maxVolume);
            return;
        }

        Serial.printf("%s - Current volume: %d, Target volume: %d, Direction: %s\n",
                      __PRETTY_FUNCTION__, currentVolume, targetVolume,
                      (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");

        int volumeDifference = abs(targetVolume - currentVolume);
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = (targetVolume > currentVolume) ? 1 : -1;

        lastFadeTime = millis();

        Serial.printf("%s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                      __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
    } else {
        Serial.printf("%s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
    }
}

//void PlayerController::displayPlayerStatusBox() {
////    Serial.println(F("    ╒════════════════════════════════════════════════════╕"));
//    Serial.println(F("    ┌─────────────────────────────────────────────────────┐"));
//    Serial.println(F("    |       --- PlayerController Status Update ---        |"));
//    Serial.println(F("    +─────────────────────────────────────────────────────+"));
//
//    Serial.printf("    │ Player type:    %-36s|\n", getPlayerTypeName());
//    Serial.printf("    │ Current volume: %-36d|\n", currentVolume);
//
//    char trackInfo[50];
//    snprintf(trackInfo, sizeof(trackInfo), "%d%s%s%s",
//             currentTrack,
//             (currentTrackName && currentTrackName[0] != '\0') ? " (" : "",
//             (currentTrackName && currentTrackName[0] != '\0') ? currentTrackName : "",
//             (currentTrackName && currentTrackName[0] != '\0') ? ")" : "");
//    Serial.printf("    │ Current track:  %-36s|\n", trackInfo);
//
//    Serial.printf("    │ Player status:  %-36s|\n", playerStatusToString(playerStatus));
//
//
//    if (playerStatus == STATUS_PLAYING) {
//        if (playDuration > 0) {
//            char durationStr[40], progressStr[40], remainingStr[40];
//
//            snprintf(durationStr, sizeof(durationStr), "%lu ms (playing)", playDuration);
//            Serial.printf("    │ Play duration:  %-36s|\n", durationStr);
//
//            unsigned long long currentTime = millis();
//            unsigned long long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
//            unsigned long long totalDuration = playDuration / 1000; // Convert to seconds
//            unsigned long long remainingTime = (elapsedTime < totalDuration) ? (totalDuration - elapsedTime) : 0;
//
//            // Create progress bar
//            const int barWidth = 26;
//            int progress = 0;
//            if (totalDuration > 0) { // Avoid division by zero
//                progress = (int)(((double)elapsedTime / totalDuration) * barWidth);
//            }
//            progress = constrain(progress, 0, barWidth); // Ensure progress is within bounds
//
//            char progressBar[barWidth + 1];
//            for (int i = 0; i < barWidth; i++) {
//                progressBar[i] = (i < progress) ? '=' : '-';
//            }
//            progressBar[barWidth] = '\0';
//
//            snprintf(progressStr, sizeof(progressStr), "[%s] %llu/%llu s", progressBar, elapsedTime, totalDuration);
//            snprintf(remainingStr, sizeof(remainingStr), "%llu s", remainingTime);
//
//            Serial.printf("    │ Progress:       %-36s|\n", progressStr);
//            Serial.printf("    │ Remaining:      %-36s|\n", remainingStr);
//        } else {
//            unsigned long elapsedTime = (millis() - playStartTime) / 1000; // Convert to seconds
//            char playbackStr[40];
//            snprintf(playbackStr, sizeof(playbackStr), "%lu s (No duration set)", elapsedTime);
//            Serial.printf("    │ Playback time:  %-36s|\n", playbackStr);
//        }
//    }
//
//    if (fadeDirection != FadeDirection::NONE) {
//        Serial.printf("    │ Fade in progress: %-32s|\n", (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
//        Serial.printf("    │ Current volume: %d, Target volume: %-17d|\n", currentVolume, targetVolume);
//    }
//
//    Serial.println(F("    └─────────────────────────────────────────────────────┘"));
//}

void PlayerController::displayPlayerStatusBox() {
    Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
    Serial.println(F("    |        --- PlayerController Status Update ---         |"));
    Serial.println(F("    +───────────────────────────────────────────────────────+"));

    Serial.printf("    │ Player type:    %-38s|\n", getPlayerTypeName());
    Serial.printf("    │ Current volume: %-38d|\n", currentVolume);

    char trackInfo[50];
    snprintf(trackInfo, sizeof(trackInfo), "%d%s%s%s",
             currentTrack,
             (currentTrackName && currentTrackName[0] != '\0') ? " (" : "",
             (currentTrackName && currentTrackName[0] != '\0') ? currentTrackName : "",
             (currentTrackName && currentTrackName[0] != '\0') ? ")" : "");
    Serial.printf("    │ Current track:  %-38s|\n", trackInfo);

    Serial.printf("    │ Player status:  %-38s|\n", playerStatusToString(playerStatus));

    if (playerStatus == STATUS_PLAYING) {
        if (playDuration > 0) {
            char durationStr[40], progressStr[40], remainingStr[40];

            snprintf(durationStr, sizeof(durationStr), "%lu ms (playing)", playDuration);
            Serial.printf("    │ Play duration:  %-38s|\n", durationStr);

            unsigned long long currentTime = millis();
            unsigned long long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
            unsigned long long totalDuration = playDuration / 1000; // Convert to seconds
            unsigned long long remainingTime = (elapsedTime < totalDuration) ? (totalDuration - elapsedTime) : 0;

            // Create progress bar
            const int barWidth = 26;
            int progress = 0;
            if (totalDuration > 0) { // Avoid division by zero
                progress = (int)(((double)elapsedTime / totalDuration) * barWidth);
            }
            progress = constrain(progress, 0, barWidth); // Ensure progress is within bounds

            char progressBar[barWidth + 1];
            for (int i = 0; i < barWidth; i++) {
                progressBar[i] = (i < progress) ? '=' : '-';
            }
            progressBar[barWidth] = '\0';

            snprintf(progressStr, sizeof(progressStr), "[%s] %llu/%llu s", progressBar, elapsedTime, totalDuration);
            snprintf(remainingStr, sizeof(remainingStr), "%llu s", remainingTime);

            Serial.printf("    │ Progress:       %-38s|\n", progressStr);
            Serial.printf("    │ Remaining:      %-38s|\n", remainingStr);
        } else {
            unsigned long elapsedTime = (millis() - playStartTime) / 1000; // Convert to seconds
            char playbackStr[40];
            snprintf(playbackStr, sizeof(playbackStr), "%lu s (No duration set)", elapsedTime);
            Serial.printf("    │ Playback time:  %-38s|\n", playbackStr);
        }
    }

    if (fadeDirection != FadeDirection::NONE) {
        Serial.printf("    │ Fade in progress: %-34s|\n", (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
        Serial.printf("    │ Current volume: %d, Target volume: %-19d|\n", currentVolume, targetVolume);
    }

    Serial.println(F("    └───────────────────────────────────────────────────────┘"));
}

void PlayerController::update() {
    unsigned long currentTime = millis();

    // TODO do a periodic update every 5 seconds and print statistics like sound playing status, current time, etc.
    // Periodic update every 5 seconds
    if (currentTime - lastPeriodicUpdate >= DISPLAY_STATUS_UPDATE_INTERVAL) {
        lastPeriodicUpdate = currentTime;

        PlayerController::displayPlayerStatusBox();

//        // Print statistics
//        Serial.println(F("    ┌────────────────────────────────────────────────────┐"));
//        Serial.println(F("    |       --- PlayerController Status Update ---       |"));
//        Serial.println(F("    └────────────────────────────────────────────────────┘"));
//           Serial.printf("    │ Player type:    %s\n", getPlayerTypeName());
//           Serial.printf("    │ Current volume: %d\n", currentVolume);
////         Serial.printf("    │ Current time:   %lu ms\n", currentTime);
//           Serial.printf("    │ Current track:  %d", currentTrack);
//          if (currentTrackName && currentTrackName[0] != '\0') {
//              Serial.printf(" (%s)", currentTrackName);
//          }
//        Serial.printf("\n");
//
//           Serial.printf("    │ Player status:  %s\n", playerStatusToString(playerStatus));
//
//
//
//if (playerStatus == STATUS_PLAYING) {
////    Serial.printf("playDuration: %lu ms (playing)\n", playDuration);
//    if (playDuration > 0) {
//          Serial.printf("    │ Play duration:  %lu ms (playing)\n", playDuration);
//
//        unsigned long long elapsedTime = (unsigned long long)(currentTime - playStartTime) / 1000; // Convert to seconds
//        unsigned long long totalDuration = (unsigned long long)playDuration / 1000; // Convert to seconds
//        unsigned long long remainingTime = (elapsedTime < totalDuration) ? (totalDuration - elapsedTime) : 0;
//
//        // Create progress bar
//        const int barWidth = 26;
//        int progress = 0;
//        if (totalDuration > 0) { // Avoid division by zero
//            progress = (int)(((double)elapsedTime / totalDuration) * barWidth);
//        }
//        progress = constrain(progress, 0, barWidth); // Ensure progress is within bounds
//
//        char progressBar[barWidth + 1];
//        for (int i = 0; i < barWidth; i++) {
//            progressBar[i] = (i < progress) ? '=' : '-';
//        }
//        progressBar[barWidth] = '\0';
//
//        Serial.printf("    │ Progress: [%s] %llu/%llu s\n", progressBar, elapsedTime, totalDuration);
//        Serial.printf("    │ Remaining: %llu s\n", remainingTime);
//    } else {
//        unsigned long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
//        Serial.printf("    │ Playback time:     %lu s (No duration set)\n", elapsedTime);
//    }
//}
//
//        if (fadeDirection != FadeDirection::NONE) {
//            Serial.printf("    │ Fade in progress: Direction: %s\n", (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
//            Serial.printf("    │ Current volume: %d, Target volume: %d\n", currentVolume, targetVolume);
//        }
//
//        Serial.println(F("    └─────────────────────────────────────────────────────"));
    }

    // Check if sound is playing and duration is set
    if (playerStatus == STATUS_PLAYING && playDuration > 0) {
        unsigned long elapsedTime = currentTime - playStartTime;


        if (elapsedTime >= playDuration) {

            PlayerController::displayPlayerStatusBox(); // Update status box

            playerStatus = STATUS_STOPPED;
            // TODO create a method for resetting the track when it finishes playing
            currentTrack = 0;
            currentTrackName = "";
            Serial.printf("%s - Sound finished playing. Duration: %lu ms, New playerStatus: %s\n",
                  __PRETTY_FUNCTION__, playDuration, playerStatusToString(playerStatus));
            playDuration = 0; // Reset playDuration

            PlayerController::displayPlayerStatusBox(); // Update status box
        }
    }

  // Handle fading
  if (fadeDirection != FadeDirection::NONE && currentTime - lastFadeTime >= fadeIntervalMs) {
      lastFadeTime = currentTime;

      int newVolume = currentVolume;
      if (fadeDirection == FadeDirection::IN) {
          newVolume = min(currentVolume + volumeStep, targetVolume);
      } else if (fadeDirection == FadeDirection::OUT) {
          newVolume = max(currentVolume - volumeStep, targetVolume);
      }

      newVolume = constrain(newVolume, MIN_VOLUME, MAX_VOLUME);

      if (newVolume != currentVolume) {
          setVolume(newVolume);
          currentVolume = newVolume;
      }

      // Check if fade is complete
      if (currentVolume == targetVolume) {
          fadeDirection = FadeDirection::NONE;
          unsigned long totalFadeTime = currentTime - fadeStartTime;
          Serial.printf("                %s - Fade %s complete. Final volume: %d, Total fade time: %lu ms\n",
                        __PRETTY_FUNCTION__,
                        (fadeDirection == FadeDirection::IN) ? "in" : "out",
                        currentVolume, totalFadeTime);

          // Check if we should stop the sound after fading
          if (shouldStopAfterFade) {
              stopSound();
              shouldStopAfterFade = false;
              Serial.printf("                %s - Sound stopped after fade completion\n", __PRETTY_FUNCTION__);
          }
      }
  }
}
