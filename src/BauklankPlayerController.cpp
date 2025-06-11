// BauklankPlayerController.cpp
#include "BauklankPlayerController.h"

#if __has_include("debug.h")
    #include "debug.h"
#endif

#include <Arduino.h>

unsigned long lastPeriodicUpdate = 0;
unsigned long PLAYER_STATUS_INTERVAL_MS = 5000;

void PlayerController::setVolume(int _volume) {
    currentVolume = constrain(_volume, MIN_VOLUME, MAX_VOLUME);
    setPlayerVolume(currentVolume);
}

int PlayerController::getVolume() {
    return currentVolume;
}

void PlayerController::playSoundRandom(int minTrack, int maxTrack) {
    int rnd = random(minTrack, maxTrack + 1);
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  🎲▶️ %s - Playing random track: %d\n", __PRETTY_FUNCTION__, rnd);
    #endif
    playSound(rnd);
}


void PlayerController::decodeFolderAndTrack(uint16_t trackNumber, uint8_t& folder, uint8_t& track) {
    folder = (trackNumber - 1) / 255 + 1;
    track = (trackNumber - 1) % 255 + 1;
}

void PlayerController::playSound(int track) {
    playerStatus = STATUS_PLAYING;
    currentTrack = track;
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - Playing track: %d, playerStatus: %s\n", __PRETTY_FUNCTION__, track, playerStatusToString(playerStatus));
    #endif
}

void PlayerController::playSound(int track, unsigned long durationMs) {

    // Add duration
    playStartTime = millis();
    playDuration = durationMs;

    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - Playing track: %d, duration: %d ms, startTime: %lu, endTime: %lu\n",
                      __PRETTY_FUNCTION__, track, durationMs, playStartTime, playStartTime + playDuration);
    #endif

    playSound(track);
}

void PlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    // Add duration
    playStartTime = millis();
    playDuration = durationMs;
    currentTrackName = trackName;

    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - Playing track: %d (%s), duration: %d ms, startTime: %lu, endTime: %lu\n",
                      __PRETTY_FUNCTION__, track, currentTrackName, durationMs, playStartTime, playStartTime + playDuration);
    #endif
    playSound(track);
}

void PlayerController::stopSound() {
    // TODO create a private method to reset the track when it is stopped
    playerStatus = STATUS_STOPPED;
    currentTrack = 0;
    currentTrackName = "";
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ⏹️ %s - Stop sound, playerStatus: %d\n", __PRETTY_FUNCTION__, playerStatus);
    #endif
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

void PlayerController::fadeIn(int durationMs, int targetVolume) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📈 %s - Fade in requested: duration %d ms, target volume %d\n", __PRETTY_FUNCTION__, durationMs, targetVolume);
    #endif

    if (fadeDirection == FadeDirection::NONE) {
        fadeStartTime = millis();
        fadeDirection = FadeDirection::IN;
        currentVolume = constrain(getVolume(), MIN_VOLUME, MAX_VOLUME);
        this->targetVolume = constrain(targetVolume, MIN_VOLUME, MAX_VOLUME);
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  🔉 %s - Current volume: %d, Target volume: %d\n", __PRETTY_FUNCTION__, currentVolume, this->targetVolume);
        #endif
        if (currentVolume >= this->targetVolume) {
            fadeDirection = FadeDirection::NONE;
            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("  📈🚫 %s - No need to fade in, current volume (%d) is already at or above target (%d)\n",
                              __PRETTY_FUNCTION__, currentVolume, this->targetVolume);
             #endif
           return;
        }

        int volumeDifference = this->targetVolume - currentVolume;
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = 1;

        lastFadeTime = millis();

        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📈🔢 %s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                          __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
         #endif
   } else {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📈 %s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
        #endif
    }
}

// TODO add duration and name.
// Or maybe do a playSound with additional fade params
void PlayerController::fadeIn(int durationMs, int targetVolume, int playTrack) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📈 %s - Fade in with track requested: duration %d ms, target volume %d, track %d\n",
                      __PRETTY_FUNCTION__, durationMs, targetVolume, playTrack);
    #endif

    // Set volume to 0 before starting playback
    setVolume(0);
    currentVolume = 0;

    // Start playing the track
    playSound(playTrack);

    // Now call the regular fadeIn method
    fadeIn(durationMs, targetVolume);
}

void PlayerController::fadeIn(int durationMs, int targetVolume, int playTrack, unsigned long trackDurationMs) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📈 %s - Fade in with track and duration requested: fade duration %d ms, target volume %d, track %d, track duration %lu ms\n",
                      __PRETTY_FUNCTION__, durationMs, targetVolume, playTrack, trackDurationMs);
    #endif

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
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📉 %s - Fade out requested: duration %d ms, target volume %d\n", __PRETTY_FUNCTION__, durationMs, targetVolume);
    #endif

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
            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("  📉🚫 %s - No need to fade out, current volume (%d) is already at or below target (%d)\n",
                              __PRETTY_FUNCTION__, currentVolume, targetVolume);
            #endif
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
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📉 %s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
        #endif
}
}

void PlayerController::fade(int durationMs, int minVolume, int maxVolume) {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  📉📈 %s - Fade requested: duration %d ms, min volume %d, max volume %d\n",
                      __PRETTY_FUNCTION__, durationMs, minVolume, maxVolume);
    #endif

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
            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("  📈📉🚫 %s - No need to fade, current volume (%d) is already within range (%d-%d)\n",
                              __PRETTY_FUNCTION__, currentVolume, minVolume, maxVolume);
             #endif
           return;
        }

        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  🔉 %s - Current volume: %d, Target volume: %d, Direction: %s\n",
                          __PRETTY_FUNCTION__, currentVolume, targetVolume,
                          (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
        #endif

        int volumeDifference = abs(targetVolume - currentVolume);
        fadeIntervalMs = constrain(durationMs / volumeDifference, 10, 500); // Interval between 10ms and 500ms
        volumeStep = (targetVolume > currentVolume) ? 1 : -1;

        lastFadeTime = millis();

        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📈📉🔢 %s - Fade calculation: Volume step: %d, Fade interval: %d ms\n",
                          __PRETTY_FUNCTION__, volumeStep, fadeIntervalMs);
        #endif
    } else {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.printf("  📈📉🚫 %s - Fade already in progress, ignoring new fade request\n", __PRETTY_FUNCTION__);
        #endif
    }
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
    static int statusUpdateCounter = 0;
    statusUpdateCounter++;

    // Reset counter when it reaches 7 digits (1,000,000)
    if (statusUpdateCounter >= 1000000) {
        statusUpdateCounter = 1;  // Reset to 1 instead of 0 to avoid showing #000000
    }

    Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
       Serial.printf("    | PlayerController Status Update                 %6d |\n", statusUpdateCounter);
    Serial.println(F("    +───────────────────────────────────────────────────────+"));

    Serial.printf("    │ Player type:    %-38s|\n", getPlayerTypeName());
    // Display current equalizer preset
    Serial.printf("    │ EQ Preset:      %-38s|\n", equalizerPresetToString(currentEqualizerPreset));

    displayVolumeProgressBar();

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
            playerStatus = STATUS_STOPPED;
            currentTrack = 0;
            currentTrackName = "";
            Serial.printf("  🏁 %s - Sound finished playing. Duration: %lu ms, New playerStatus: %s\n",
                  __PRETTY_FUNCTION__, playDuration, playerStatusToString(playerStatus));
            playDuration = 0; // Reset playDuration
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
            #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                Serial.printf("                %s - Fade %s complete. Final volume: %d, Total fade time: %lu ms\n",
                              __PRETTY_FUNCTION__,
                              (fadeDirection == FadeDirection::IN) ? "in" : "out",
                              currentVolume, totalFadeTime);
            #endif

            // Check if we should stop the sound after fading
            if (shouldStopAfterFade) {
                stopSound();
                shouldStopAfterFade = false;
                #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
                    Serial.printf("                %s - Sound stopped after fade completion\n", __PRETTY_FUNCTION__);
                #endif
            }
        }
    }

    // Check if player status has changed
    #if DISPLAY_PLAYER_STATUS_ENABLED == true
    if (playerStatus != lastPlayerStatus) {
      Serial.println(F("    ┌───────────────────────────────────────────────────────┐"));
         Serial.printf("    | playerStatus change from %s => %s\n", playerStatusToString(lastPlayerStatus), playerStatusToString(playerStatus));
        displayPlayerStatusBox();
        lastPlayerStatus = playerStatus;
    }
    #endif
}
