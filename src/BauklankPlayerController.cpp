// PlayerController.cpp
#include "BauklankPlayerController.h"
#include <Arduino.h>

void PlayerController::setVolume(int _volume) {
    currentVolume = _volume;
    int _playerVolume = map(_volume, 0, 100, 0, 30);
//    Serial.printf("%s - Setting volume to %d (mapped from %d)\n", __PRETTY_FUNCTION__, _playerVolume, _volume);
    setPlayerVolume(_playerVolume);
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
    Serial.printf("%s - Playing track: %d, playerStatus: %s\n", __PRETTY_FUNCTION__, track, playerStatusToString(playerStatus));
}

void PlayerController::playSound(int track, uint32_t durationMs) {
    playSound(track);

    // Add duration
    playStartTime = millis();
    playDuration = durationMs;

    Serial.printf("%s - Playing track: %d, duration: %d ms, startTime: %lu, endTime: %lu\n",
                  __PRETTY_FUNCTION__, track, durationMs, playStartTime, playStartTime + playDuration);
}

void PlayerController::stopSound() {
    playerStatus = STATUS_STOPPED;
    Serial.printf("%s - Stopping sound, playerStatus: %d\n", __PRETTY_FUNCTION__, playerStatus);
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

void PlayerController::update() {
    unsigned long currentTime = millis();

    // TODO do a periodic update every 5 seconds and print statistics like sound playing status, current time, etc.
    // Periodic update every 5 seconds
    static unsigned long lastPeriodicUpdate = 0;
    if (currentTime - lastPeriodicUpdate >= 5000) {
        lastPeriodicUpdate = currentTime;

        // Print statistics
        Serial.println(F("\n          --- PlayerController Status Update ---"));
        Serial.println(F("        ┌─────────────────────────────────────────────────────"));
        Serial.printf("        │ Player type:    %s\n", getPlayerTypeName());
        Serial.printf("        │ Current time:   %lu ms\n", currentTime);
        Serial.printf("        │ Player status:  %s\n", playerStatusToString(playerStatus));
        int playerVolume = map(currentVolume, 0, 100, 0, 30);
        Serial.printf("        │ Current volume: %d (%d)\n", currentVolume, playerVolume);

//        if (playerStatus == STATUS_PLAYING) {
//            if (playDuration > 0) {
//                unsigned long elapsedTime = currentTime - playStartTime;
//                unsigned long remainingTime = (elapsedTime < playDuration) ? (playDuration - elapsedTime) : 0;
//                Serial.printf("        │ Playback progress: %lu ms / %lu ms\n", elapsedTime, playDuration);
//                Serial.printf("        │ Remaining time: %lu ms\n", remainingTime);
//            } else {
//                Serial.printf("        │ Playback time: %lu ms (No duration set)\n", currentTime - playStartTime);
//            }
//        }

        if (playerStatus == STATUS_PLAYING) {
            if (playDuration > 0) {
                unsigned long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
                unsigned long totalDuration = playDuration / 1000; // Convert to seconds
                unsigned long remainingTime = (elapsedTime < totalDuration) ? (totalDuration - elapsedTime) : 0;

                // Create progress bar
                const int barWidth = 26;
                int progress = (elapsedTime * barWidth) / totalDuration;
                char progressBar[barWidth + 1];
                for (int i = 0; i < barWidth; i++) {
                    progressBar[i] = (i < progress) ? '=' : '-';
                }
                progressBar[barWidth] = '\0';

                Serial.printf("        │ Progress: [%s] %lu/%lu s\n", progressBar, elapsedTime, totalDuration);
                Serial.printf("        │ Remaining: %lu s\n", remainingTime);

//                Serial.printf("        │ Playback progress: %lu s / %lu s\n", elapsedTime, totalDuration);
//                Serial.printf("        │ Remaining time:    %lu s\n", remainingTime);
            } else {
                unsigned long elapsedTime = (currentTime - playStartTime) / 1000; // Convert to seconds
                Serial.printf("        │ Playback time:     %lu s (No duration set)\n", elapsedTime);
            }
        }

        if (fadeDirection != FadeDirection::NONE) {
            Serial.printf("        │ Fade in progress: Direction: %s\n", (fadeDirection == FadeDirection::IN) ? "IN" : "OUT");
            Serial.printf("        │ Current volume: %d, Target volume: %d\n", currentVolume, targetVolume);
        }

        Serial.println(F("        └─────────────────────────────────────────────────────"));
    }

    // Check if sound is playing and duration is set
    if (playerStatus == STATUS_PLAYING && playDuration > 0) {
        unsigned long elapsedTime = currentTime - playStartTime;
        if (elapsedTime >= playDuration) {
            playerStatus = STATUS_STOPPED;
            Serial.printf("%s - Sound finished playing. Duration: %lu ms, New playerStatus: %s\n",
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
//            Serial.printf("Updating volume: Old: %d, New: %d\n", currentVolume, newVolume);
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
        } else {
//            Serial.printf("Fade progress: Current: %d, Target: %d, Elapsed: %lu ms\n",
//                          currentVolume, targetVolume, currentTime - fadeStartTime);
        }
    }
}