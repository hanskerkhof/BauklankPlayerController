// BauklankPlayerController.cpp
#include "BauklankPlayerController.h"

#if __has_include("debug.h")
    #include "debug.h"
#endif

#include <Arduino.h>

unsigned long lastPeriodicUpdate = 0;
unsigned long PLAYER_STATUS_INTERVAL_MS = 5000;

/**
 * @brief Sets the volume of the player.
 *
 * This method sets the volume of the player to the specified level. It ensures that
 * the volume is within the allowed range by constraining it between MIN_VOLUME and MAX_VOLUME.
 *
 * @param _volume The desired volume level to set.
 *
 * @details
 * - The input volume is constrained between MIN_VOLUME and MAX_VOLUME.
 * - After constraining, it updates the currentVolume member variable.
 * - Calls setPlayerVolume() with the constrained volume to actually set the volume on the player hardware.
 *
 * @note This method uses the constrain() function to ensure the volume is within valid bounds.
 * @note The actual implementation of setting the volume on the hardware is done in setPlayerVolume().
 *
 * @see setPlayerVolume
 * @see MIN_VOLUME
 * @see MAX_VOLUME
 */
void PlayerController::setVolume(int _volume) {
    currentVolume = constrain(_volume, MIN_VOLUME, MAX_VOLUME);
    setPlayerVolume(currentVolume);
    // displayPlayerStatusBox();
}

/**
 * @brief Retrieves the current volume level of the player.
 *
 * This method returns the current volume setting of the player controller.
 * The volume is represented as an integer value.
 *
 * @return int The current volume level.
 *
 * @note The returned value is typically within the range defined by MIN_VOLUME and MAX_VOLUME,
 *       as set in the setVolume method. However, this method does not perform any range checking.
 *
 * @see setVolume
 * @see MIN_VOLUME
 * @see MAX_VOLUME
 */
int PlayerController::getVolume() {
    return currentVolume;
}

/**
 * @brief Decodes a track number into folder and track components.
 *
 * This method takes a 16-bit track number and decodes it into separate folder and track numbers.
 * It's designed to work with a file system where tracks are organized into folders, each containing
 * up to 255 tracks.
 *
 * @param trackNumber The 16-bit track number to be decoded.
 * @param folder Reference to a uint8_t where the decoded folder number will be stored.
 * @param track Reference to a uint8_t where the decoded track number will be stored.
 *
 * @details
 * - The folder number is calculated as: (trackNumber - 1) / 255 + 1
 * - The track number within the folder is calculated as: (trackNumber - 1) % 255 + 1
 * - This method assumes that folder and track numbering starts at 1, not 0.
 *
 * @note This method is useful for audio players that organize tracks in a folder structure,
 *       where each folder can contain up to 255 tracks.
 *
 * @example
 * uint16_t trackNumber = 257;
 * uint8_t folder, track;
 * decodeFolderAndTrack(trackNumber, folder, track);
 * // Result: folder = 2, track = 1
 */
void PlayerController::decodeFolderAndTrack(uint16_t trackNumber, uint8_t& folder, uint8_t& track) {
    folder = (trackNumber - 1) / 255 + 1;
    track = (trackNumber - 1) % 255 + 1;
}

void PlayerController::playSoundSetStatus(int track, unsigned long durationMs, const char* trackName) {
    playerStatus = STATUS_PLAYING;
    currentTrack = track;
    // Add duration
    if (durationMs > 0) {
        playStartTime = millis();
        playDuration = durationMs;
    } else {
        playDuration = 0; // or some default value, or keep the previous value
//        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  ⚠️ %s - Warning: durationMs is 0 or negative (%lu). Using default duration.\n", __PRETTY_FUNCTION__, durationMs);
//        #endif
    }
    if (trackName != nullptr && trackName[0] != '\0') {
        currentTrackName = trackName;
    } else {
        currentTrackName = ""; // Set to empty string if trackName is null or empty
//        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  ⚠️ %s - Warning: trackName is null or empty. Using empty string.\n", __PRETTY_FUNCTION__);
//        #endif
    }
//    currentTrackName = trackName;

//    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - Playing track: %d (%s), duration: %d ms, startTime: %lu, endTime: %lu\n",
                      __PRETTY_FUNCTION__, track, currentTrackName, durationMs, playStartTime, playStartTime + playDuration);
//    #endif
    displayPlayerStatusBox();
}

void PlayerController::stopSoundSetStatus() {
    // TODO create a private method to reset the track when it is stopped
    playerStatus = STATUS_STOPPED;
    currentTrack = 0;
    currentTrackName = "";
    playDuration = 0; // Reset playDuration
//    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ⏹️ %s - PlayerStatus: %d\n", __PRETTY_FUNCTION__, playerStatus);
//    #endif
    displayPlayerStatusBox();
}

void PlayerController::setEqualizerPreset(EqualizerPreset preset) {
    // Default implementation
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  🎚️ %s - Setting equalizer preset: ", __PRETTY_FUNCTION__);
    #endif
    currentEqualizerPreset = preset;
    // Note: This default implementation doesn't actually change any settings on the player.
    // Derived classes should override this method to implement actual functionality to set the EQ on the player.
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
    switch (preset) {
        case EqualizerPreset::NORMAL:
            Serial.println("NORMAL");
            break;
        case EqualizerPreset::POP:
            Serial.println("POP");
            break;
        case EqualizerPreset::ROCK:
            Serial.println("ROCK");
            break;
        case EqualizerPreset::JAZZ:
            Serial.println("JAZZ");
            break;
        case EqualizerPreset::CLASSIC:
            Serial.println("CLASSIC");
            break;
        case EqualizerPreset::BASS:
            Serial.println("BASS");
            break;
        default:
            Serial.println("Unknown");
            break;
    }
    #endif
}

void PlayerController::fadeIn(int durationMs, int targetVolume, int playTrack, unsigned long trackDurationMs, const char* trackName) {
    // Ensure duration is not less than the minimum
    durationMs = max(durationMs, MIN_FADE_DURATION_MS);

        // #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
    Serial.printf("  📈 %s - Fade in with track and duration requested: fade duration %d ms, target volume %d, track %d, track duration %lu ms, track name: %s\n",
                  __PRETTY_FUNCTION__, durationMs, targetVolume, playTrack, trackDurationMs, trackName);
        // #endif

    // TODO Stop any playing sound....
    if(isSoundPlaying()) {
        stopSound();
    } else {
        Serial.printf("  !-> No sound was playing\n");
    }
    // Set volume to 0 before starting playback
    setVolume(0);

    // Start playing the track with the specified duration and name
    // Call the virtual function to play the track
    playSound(playTrack, trackDurationMs, trackName);
    Serial.printf("  !-> After playing the track\n");
    delay(60); // give some time to start the sound
    Serial.printf("  !-> After the delay sound should have started\n");

    Serial.printf("%s - FadeDirection: %d\n", __PRETTY_FUNCTION__, fadeDirection);
    // Now set up the fade in
    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        fadeDirection = FadeDirection::IN;
        Serial.printf("%s - FadeDirection: %d\n", __PRETTY_FUNCTION__, fadeDirection);
        Serial.printf("%s - Current volume: %d, Target volume: %d\n", __PRETTY_FUNCTION__, currentVolume, this->targetVolume);

        this->targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);

        int volumeDifference = this->targetVolume - currentVolume;
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        Serial.printf("  📈🔢 %s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                      __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
    } else {
        Serial.printf("  📈 %s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
    }
}

void PlayerController::fadeOut(int durationMs, int targetVolume = 0, bool stopSound = true) {
    // Ensure duration is not less than the minimum
    durationMs = max(durationMs, MIN_FADE_DURATION_MS);
    shouldStopAfterFade = stopSound;

//    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📉 %s - should stop after fade: %s\n", __PRETTY_FUNCTION__, shouldStopAfterFade? "true" : "false"  );
        Serial.printf("  📉 %s - Fade out requested: duration %d ms, target volume %d\n", __PRETTY_FUNCTION__, durationMs, targetVolume);
//    #endif

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        fadeDirection = FadeDirection::OUT;
        currentVolume = constrain(getVolume(), MIN_VOLUME, MAX_VOLUME);
        this->targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);

        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  🔉 %s - Current volume: %d, Target volume: %d\n", __PRETTY_FUNCTION__, currentVolume, targetVolume);
        #endif

        if (currentVolume <= targetVolume) {
            fadeDirection = FadeDirection::NONE;
//            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("  📉🚫 %s - No need to fade out, current volume (%d) is already at or below target (%d)\n",
                                __PRETTY_FUNCTION__, currentVolume, targetVolume);
//            #endif
            return;
        }

        int volumeDifference = currentVolume - targetVolume;
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📉🔢 %s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                            __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
        #endif
    } else {
//        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📉 %s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
//        #endif
    }
}

void PlayerController::fadeTo(int durationMs, int targetVolume) {
    // Ensure duration is not less than the minimum
    durationMs = max(durationMs, MIN_FADE_DURATION_MS);

    // Constrain target volume to valid range
    targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);

    Serial.printf("  🎚️ %s - Fade to volume requested: duration %d ms, target volume %d\n",
                  __PRETTY_FUNCTION__, durationMs, targetVolume);

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        currentVolume = constrain(getVolume(), MIN_VOLUME, MAX_VOLUME);
        this->targetVolume = targetVolume;

        Serial.printf("  🔉 %s - Current volume: %d, Target volume: %d\n",
                      __PRETTY_FUNCTION__, currentVolume, targetVolume);

        if (currentVolume < targetVolume) {
            fadeDirection = FadeDirection::IN;
        } else if (currentVolume > targetVolume) {
            fadeDirection = FadeDirection::OUT;
        } else {
            Serial.printf("  🔉 %s - Current volume is already at target. No fade needed.\n",
                          __PRETTY_FUNCTION__);
            return;
        }

        int volumeDifference = abs(targetVolume - currentVolume);
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        Serial.printf("  🎚️🔢 %s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                      __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
    } else {
        Serial.printf("  🎚️ %s - Fade already in progress, ignoring new fade request\n",
                      __PRETTY_FUNCTION__);
    }
    displayPlayerStatusBox();
}




// Helper function to create a progress bar string
const char* PlayerController::createProgressBar(int value, int maxLength) {
    maxLength = constrain(maxLength, 3, 100);
    static char bar[101]; // Maximum possible size + null terminator
    int barLength = maxLength - 2; // Subtract 2 for the brackets

    bar[0] = '[';
    int filledLength = (value * barLength) / 100; // Assuming value is 0-100
    for (int i = 0; i < barLength; i++) {
        bar[i + 1] = (i < filledLength) ? '=' : '-';
    }
    bar[maxLength - 1] = ']';
    bar[maxLength] = '\0'; // Null-terminate the string

    return bar;
}

void PlayerController::displayEqualizerSettings() {
    Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
    Serial.println(F("    |           --- Equalizer Settings ---                  |"));
    Serial.println(F("    +───────────────────────────────────────────────────────+"));

    // Display current equalizer preset
    Serial.printf("    │ Current Preset: %-38s|\n", equalizerPresetToString(currentEqualizerPreset));

    // Display individual band settings
    // Note: You may need to adjust these based on your actual equalizer implementation
    Serial.println(F("    │                                                       |"));
    Serial.println(F("    │ Band Settings:                                        |"));
    Serial.printf("    │   Bass:   [%-10s] %-15d|\n", createProgressBar(bassLevel, 26), bassLevel);
    Serial.printf("    │   Mid:    [%-10s] %-15d|\n", createProgressBar(midLevel, 26), midLevel);
    Serial.printf("    │   Treble: [%-10s] %-15d|\n", createProgressBar(trebleLevel, 26), trebleLevel);

    Serial.println(F("    └───────────────────────────────────────────────────────┘"));
}

// Helper function to convert EqualizerPreset to string
const char* PlayerController::equalizerPresetToString(EqualizerPreset preset) {
    switch (preset) {
        case EqualizerPreset::NORMAL: return "NORMAL";
        case EqualizerPreset::POP: return "POP";
        case EqualizerPreset::ROCK: return "ROCK";
        case EqualizerPreset::JAZZ: return "JAZZ";
        case EqualizerPreset::CLASSIC: return "CLASSIC";
        case EqualizerPreset::BASS: return "BASS";
        default: return "Unknown";
    }
}

void PlayerController::displayVolumeProgressBar() {
    const int volumeBarWidth = 26;
    int volumePercentage = (currentVolume * 100) / MAX_VOLUME;
    const char* volumeProgressBar = createProgressBar(volumePercentage, volumeBarWidth);
    Serial.printf("    │ Volume:         %s %2d/%-2d      |\n", volumeProgressBar, currentVolume, MAX_VOLUME);
}

void PlayerController::displayPlayerStatusBox() {
    const int STATUS_FIELD_WIDTH = 44;
    static int statusUpdateCounter = 0;
    statusUpdateCounter++;

    // Reset counter when it reaches 7 digits (1,000,000)
    if (statusUpdateCounter >= 1000000) {
        statusUpdateCounter = 1;  // Reset to 1 instead of 0 to avoid showing #000000
    }

    Serial.println(F("    ┌─────────────────────────────────────────────────────────────┐"));
       Serial.printf("    | PlayerController Status Update                       %6d |\n", statusUpdateCounter);
    Serial.println(F("    +─────────────────────────────────────────────────────────────+"));

    // TODO move these to a separate function for better readability
    Serial.printf("    │ Player type:    %-44s|\n", getPlayerTypeName());
    // Display current equalizer preset
    Serial.printf("    │ EQ Preset:      %-44s|\n", equalizerPresetToString(currentEqualizerPreset));

    displayVolumeProgressBar();

    char trackInfo[50];
    snprintf(trackInfo, sizeof(trackInfo), "%d%s%s%s",
             currentTrack,
             (currentTrackName && currentTrackName[0] != '\0') ? " (" : "",
             (currentTrackName && currentTrackName[0] != '\0') ? currentTrackName : "",
             (currentTrackName && currentTrackName[0] != '\0') ? ")" : "");
    Serial.printf("    │ Current track:  %-*s|\n", STATUS_FIELD_WIDTH, trackInfo);
    Serial.printf("    │ Player status:  %-*s|\n", STATUS_FIELD_WIDTH, playerStatusToString(playerStatus));

    if (playerStatus == STATUS_PLAYING) {
        if (playDuration > 0) {
            char durationStr[40], progressStr[40], remainingStr[40];

            snprintf(durationStr, sizeof(durationStr), "%lu ms (playing)", playDuration);
            Serial.printf("    │ Play duration:  %-44s|\n", durationStr);

            unsigned long long currentTime = millis();
            unsigned long long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
            unsigned long long totalDuration = playDuration / 1000; // Convert to seconds
            unsigned long long remainingTime = (elapsedTime < totalDuration) ? (totalDuration - elapsedTime) : 0;

            const int barWidth = 26;
            int progress = 0;
            if (totalDuration > 0) { // Avoid division by zero
                progress = (int)(((double)elapsedTime / totalDuration) * barWidth);
            }
            progress = constrain(progress, 0, barWidth); // Ensure progress is within bounds
            int progressPercentage = (totalDuration > 0) ? (int)(((double)elapsedTime / totalDuration) * 100) : 0;
            const char* progressBar = createProgressBar(progressPercentage, barWidth);
            snprintf(progressStr, sizeof(progressStr), "%s %llu/%llu s", progressBar, elapsedTime, totalDuration);
            snprintf(remainingStr, sizeof(remainingStr), "%llu s", remainingTime);

            Serial.printf("    │ Progress:       %-44s|\n", progressStr);
            Serial.printf("    │ Remaining:      %-44s|\n", remainingStr);




        } else {
            unsigned long elapsedTime = (millis() - playStartTime) / 1000; // Convert to seconds
            char playbackStr[40];
            snprintf(playbackStr, sizeof(playbackStr), "%lu s (No duration set)", elapsedTime);
            Serial.printf("    │ Playback time:  %-44s|\n", playbackStr);
        }
    }

    if (fadeDirection != FadeDirection::NONE) {
        Serial.printf("    │ Fade direction: %-44s|\n", (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
        Serial.printf("    │ Current volume: %d, Target volume: %-44d|\n", currentVolume, targetVolume);
    }

    Serial.println(F("    └─────────────────────────────────────────────────────────────┘"));
}

void PlayerController::update() {
    unsigned long currentTime = millis();
    // Define a static variable lastPlayerStatus to store the last player status
    // This variable retains its value between function calls
    // It is only accessible within this update() method
    // It is initialized only once, when the function is first called
    static PlayerStatus lastPlayerStatus = STATUS_STOPPED;

    // Periodic update
    #if DISPLAY_PLAYER_STATUS_PERIODIC == true
    if (currentTime - lastPeriodicUpdate >= PLAYER_STATUS_INTERVAL_MS) {
        lastPeriodicUpdate = currentTime;
        // displayEqualizerSettings();
      if (playerStatus != STATUS_STOPPED) {
        Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
           Serial.printf("    | Periodic Update (every %3d seconds)                   |\n", PLAYER_STATUS_INTERVAL_MS / 1000);
        displayPlayerStatusBox();
      }
    }
    #endif

    // Check if sound is playing and duration is set
    if (playerStatus == STATUS_PLAYING && playDuration > 0) {
        unsigned long elapsedTime = currentTime - playStartTime;

        if (elapsedTime >= playDuration) {
//            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("  🏁 %s - Sound finished playing. Duration: %lu ms, New playerStatus: %s\n",
                      __PRETTY_FUNCTION__, playDuration, playerStatusToString(playerStatus));
//            #endif
            stopSoundSetStatus();
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
            // Log the volume change for debugging
//           Serial.printf("  FADE - Changing volume from %d to %d (Target: %d, Direction: %s)\n",
//                          currentVolume, newVolume, targetVolume,
//                          fadeDirection == FadeDirection::IN ? "IN" : "OUT");
            setVolume(newVolume);
            currentVolume = newVolume;
        }

        // Check if fade is complete
        if (currentVolume == targetVolume) {
            // Check if we need to stop the sound before resetting fadeDirection
            if (fadeDirection == FadeDirection::OUT && shouldStopAfterFade) {
                Serial.println("  FADE - Stopping sound after fade out");
                stopSound();
                shouldStopAfterFade = false; // Reset the flag
            }

            // Now set fadeDirection to NONE
            FadeDirection previousFadeDirection = fadeDirection;
            fadeDirection = FadeDirection::NONE;

            fadeDirection = FadeDirection::NONE;
            unsigned long totalFadeTime = currentTime - fadeStartTime;
            Serial.printf("  FADE - Fade complete. Final volume: %d, Total fade time: %lu ms\n",
                          currentVolume, totalFadeTime);

        }
    }

    // Check if player status has changed
    #if DISPLAY_PLAYER_STATUS_ENABLED == true
    if (playerStatus != lastPlayerStatus) {
        Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
        char statusChangeStr[53]; // 53 characters + null terminator
        snprintf(statusChangeStr, sizeof(statusChangeStr),
            "Status from %-12s => %-12s",
            playerStatusToString(lastPlayerStatus),
            playerStatusToString(playerStatus));
        Serial.printf("    | %-53s |\n", statusChangeStr);
        displayPlayerStatusBox();
        lastPlayerStatus = playerStatus;
    }
    #endif
}
