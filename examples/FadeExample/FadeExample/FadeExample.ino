#include "AnalogReader.h"

#define MD_PLAYER_ENABLED
// #define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED

#include <BauklankPlayerController.h>

#define MD_PLAYER_RX_PIN D1
#define MD_PLAYER_TX_PIN D2

#ifdef MD_PLAYER_ENABLED
#include <MDPlayerController.h>
MDPlayerController player(MD_PLAYER_RX_PIN, MD_PLAYER_TX_PIN);
#elif defined(DY_PLAYER_ENABLED)
#include <DYPlayerController.h>
DYPlayerController player(DY_PLAYER_RX_PIN, DY_PLAYER_TX_PIN);
#elif defined(DF_PLAYER_ENABLED)
#include <DFRobotPlayerController.h>
DFRobotPlayerController player(DF_PLAYER_RX_PIN, DF_PLAYER_TX_PIN);
#endif

enum PlayerStates {
  STOP_TRACK,
  FADE_IN,
  FADE_OUT,
  PLAY_TRACK,
  FADE_TO_LOW,
  FADE_TO_HIGH,
  // STOP_TRACK,
  // FADE_IN,
  // FADE_OUT,
  NUM_STATES  // This will automatically be set to the number of states
};
// NOTE:
// NUM_STATES will have a value of 9, which is the total number of states defined.
// This technique is useful because:
// 1. It automatically updates when you add or remove states.
// 2. It helps prevent off-by-one errors when cycling through states.
// 3. It makes the code more maintainable, as you don't need to manually update a count when changing the enum.

PlayerStates currentState = PLAY_TRACK;
int playerStateIndex = 0;
unsigned long stateStartTime = 0;
const unsigned long STATE_DURATION = 6000;  // 8 seconds per state
const int FADE_DURATION_MS = 3000;
const int FADE_IN_TARGET_VOLUME = 100;
const int FADE_OUT_TARGET_VOLUME = 0;

#define MIN_SOUND_INDEX 1
#define MAX_SOUND_INDEX 1
int soundIndex = MIN_SOUND_INDEX;
uint8_t playerVolume;
// Define minutes and seconds
const uint8_t minutes = 2;
const uint8_t seconds = 3;
static const unsigned long trackDurationMs = ((minutes * 60) + seconds) * 1000;

#define PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS 8000
uint32_t _plan_tickCount;
uint32_t _plan_startMillis;
uint32_t _plan_currentMillis;
uint32_t _plan_targetNextMessageTime;
const uint32_t LOOP_TICK_INTERVAL_MS = 1000;

int VOLUME_ANALOG_NUM_SAMPLES = 10;
int initialVolume = 19;
AnalogReader analogReader(
  /*pin=*/A0,
  /*minValue=*/0,
  /*maxValue=*/30,
  /*readInterval=*/3,
  /*numSamples=*/VOLUME_ANALOG_NUM_SAMPLES,
  /*debounceInterval=*/50  // Must be greater than 40 to work because the player.setVolume command takes a minimum of 40ms
);

void onAnalogValueChanged(int newValue) {
  Serial.printf("🔊 [%s] Analog value changed to: %d\n", __PRETTY_FUNCTION__, newValue);
  playerVolume = newValue;
  player.setVolume(playerVolume);
  // Add your logic here to handle the new value
  // Here you can update your volume or perform any other action
}

void fadeInTrack(int track, const char* trackName) {
    // int steps = random(0, 5); // 0 to 4 inclusive
    // int rnd = 1000 + (steps * 500); // This will give values: 1000, 1500, 2000, 2500, 3000
    // int rnd = 400;
    // Serial.printf("Fade in duration: %d ms\n", rnd);
    // player.fadeIn(rnd, FADE_IN_TARGET_VOLUME, track, trackDurationMs, trackName);
    player.fadeIn(FADE_DURATION_MS, FADE_IN_TARGET_VOLUME, track, trackDurationMs, trackName);
}

void fadeOutTrack(bool stopAfterFade, const char* trackName) {
    // int steps = random(0, 5); // 0 to 4 inclusive
    // int rnd = 1000 + (steps * 500); // This will give values: 1000, 1500, 2000, 2500, 3000
    // int rnd = 400;
    // Serial.printf("Fade in duration: %d ms\n", rnd);
    // player.fadeOut(rnd, FADE_OUT_TARGET_VOLUME, stopAfterFade);
    player.fadeOut(FADE_DURATION_MS, FADE_OUT_TARGET_VOLUME, stopAfterFade);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  Serial.println(__FILE__);
  Serial.println("Compiled " __DATE__ " of " __TIME__);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  // Start the player before the analogReader.
  player.begin();
  // No need to set volume here, as it's already set by the callback
  // player.setVolume(volume);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  analogReader.setCallback(onAnalogValueChanged);
  analogReader.begin();
  Serial.println(F("--------------------------------------------------------------------------------------------"));

  Serial.println(F("--------------------------------------------------------------------------------------------"));
}

void loop() {
  _plan_currentMillis = millis();

  player.update();
  analogReader.update();

  //  // Sound playing logic
  //  if (!player.isSoundPlaying()) {
  //    Serial.printf("🔇 [%s] Not playing\n", __PRETTY_FUNCTION__);
  //    Serial.printf("🔊 [%s] Setting volume to: %d\n", __PRETTY_FUNCTION__, playerVolume);
  //    player.setVolume(playerVolume);
  //    Serial.printf("▶️ [%s] Playing sound index: %d, duration: %d ms\n", __PRETTY_FUNCTION__, soundIndex, trackDurationMs);
  //    player.playSound(soundIndex, trackDurationMs, "TRACK_NAME");
  //
  //    // Increment soundIndex and reset if it exceeds MAX_SOUND_INDEX
  //    soundIndex++;
  //    if (soundIndex > MAX_SOUND_INDEX) {
  //      soundIndex = MIN_SOUND_INDEX;
  //    }
  //
  //    delay(20);  // wait a bit so we are sure that the sound has started
  //  }

  if (_plan_currentMillis - stateStartTime >= STATE_DURATION) {
    // Time to move to the next state
    stateStartTime = _plan_currentMillis;

    // Move to the next state
    playerStateIndex = (playerStateIndex + 1) % NUM_STATES;

    PlayerStates currentState = static_cast<PlayerStates>(playerStateIndex);

    Serial.printf("Changing to state index: %d\n", playerStateIndex);

    switch (currentState) {
      // case NOOP:
        player.stop();
      //   Serial.println("State: NOOP");
      //   // Do nothing in this state, just wait
      //   break;

      case PLAY_TRACK:
        Serial.println("\nState: PLAY_TRACK\n\n");
        player.stop();
        player.setVolume(100);
        player.playSound(soundIndex, trackDurationMs, "TRACK_NAME");
        Serial.printf("Playing sound index: %d, duration: %d ms\n", soundIndex, trackDurationMs);
        break;

    case FADE_TO_LOW:
        Serial.printf("\nState: FADE_TO_LOW\n\n");
        
        player.setVolume(100);
        // player.fadeTo(FADE_DURATION_MS, 14);
        player.fadeTo(700, 18);
        break;

    case FADE_TO_HIGH:
        Serial.printf("\nState: FADE_TO_HIGH\n\n");
        player.fadeTo(2000, 100);
        break;

      case STOP_TRACK:
        Serial.printf("\nState: STOP_TRACK\n\n");
        player.stop();
        // Serial.println("Stopped playing sound");
        break;

//      case STOP_TRACK2:
//        Serial.println("State: STOP_TRACK2");
//        player.stop();
//        Serial.println("Stopped playing sound (alternative method)");
//        // You can add additional actions here, such as:
//        // - Resetting some variables
//        // - Logging additional information
//        // - Triggering other events
//        break;

        case FADE_IN:
        Serial.printf("\nState: FADE_IN\n\n");
            fadeInTrack(soundIndex, "FADE_IN_TRACK_NAME");
            break;

      case FADE_OUT:
        Serial.printf("\nState: FADE_OUT\n\n");
        fadeOutTrack(true, "FADE_OUT_TRACK_NAME");
        break;

      // case FADE_OUT2:
      //   Serial.println("State: FADE_OUT2");
      //   // This could be an alternative fade out, for example:
      //   // - Fading out to a non-zero volume
      //   // - Using a different duration
      //   // - Performing additional actions during or after the fade
      //   // player.fadeOut(FADE_DURATION / 2, playerVolume / 2);  // Faster fade to half volume
      //   Serial.println("Fading out (alternative method)");
      //   // You could add additional actions here, such as:
      //   // - Scheduling a complete stop after the fade
      //   // - Triggering other events
      //   break;


        // case FADE_IN_START_TRACK:
        //     Serial.println("State: FADE_IN_START_TRACK");
        //     soundIndex = (soundIndex >= MAX_SOUND_INDEX) ? MIN_SOUND_INDEX : soundIndex + 1;
        //     fadeInTrack(soundIndex, "FADE_IN_START_TRACK_NAME");
        //     break;

      // case FADE_IN_STOP_TRACK:
      //   Serial.println("State: FADE_OUT_STOP_TRACK");
      //   soundIndex = (soundIndex >= MAX_SOUND_INDEX) ? MIN_SOUND_INDEX : soundIndex + 1;
      //   fadeOutTrack(true, "FADE_OUT_STOP_TRACK_NAME");
      //   break;

      default:
        Serial.printf("\nUnknown state\n\n");
        break;
    }

    // Serial.printf("Current time: %lu, Next state change: %lu\n",
                  // _plan_currentMillis, stateStartTime + STATE_DURATION);
  }  // state interval

  if (_plan_currentMillis > _plan_targetNextMessageTime) {
    _plan_tickCount++;
    _plan_targetNextMessageTime = _plan_currentMillis + PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS;
    // Serial.printf("plan_update() :: _plan_currentMillis: %d, _plan_targetNextMessageTime: %d, _plan_tickCount: %d\n", _plan_currentMillis, _plan_targetNextMessageTime, _plan_tickCount);
  }
}  // Loop
