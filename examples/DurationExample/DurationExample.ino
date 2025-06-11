#include "debug.h"

// #define MD_PLAYER_ENABLED
#define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED

// Include the base class for the player controller
#include <BauklankPlayerController.h>
//using EqualizerPreset = PlayerController::EqualizerPreset;

#ifdef MD_PLAYER_ENABLED
// #define MD_PLAYER_RX_PIN D1
// #define MD_PLAYER_TX_PIN D2
#define MD_PLAYER_RX_PIN 0  // green
#define MD_PLAYER_TX_PIN 2  // blue
// Include the specific class for the player controller
#include <MDPlayerController.h>
// Instantiate the MDPlayerController with the RX and TX pins
MDPlayerController player(MD_PLAYER_RX_PIN, MD_PLAYER_TX_PIN);
#elif defined(DY_PLAYER_ENABLED)
// #define DY_PLAYER_RX_PIN D1
// #define DY_PLAYER_TX_PIN D2
#define DY_PLAYER_RX_PIN 0  // green
#define DY_PLAYER_TX_PIN 2  // blue
// Include the specific class for the player controller
#include <DYPlayerController.h>
// Instantiate the DYPlayerController with the RX and TX pins
DYPlayerController player(DY_PLAYER_RX_PIN, DY_PLAYER_TX_PIN);
#elif defined(DF_PLAYER_ENABLED)
// #define DF_PLAYER_RX_PIN D1
// #define DF_PLAYER_TX_PIN D2
#define DF_PLAYER_RX_PIN 0  // green
#define DF_PLAYER_TX_PIN 2  // blue
// Include the specific class for the player controller
#include <DFRobotPlayerController.h>
// Instantiate the DFRobotPlayerController with the RX and TX pins
DFRobotPlayerController player(DF_PLAYER_RX_PIN, DF_PLAYER_TX_PIN);
#endif

const int MIN_SOUND_INDEX = 5;
const int MAX_SOUND_INDEX = 8;
int soundIndex = MIN_SOUND_INDEX;
uint8_t playerVolume = 22;
// Define minutes and seconds for the duration of the track
const uint8_t minutes = 0;
const uint8_t seconds = 3;
// const uint8_t seconds = 20;
static const uint32_t trackDurationMs = ((minutes * 60) + seconds) * 1000;
char trackName[20];  // Adjust the size as needed

#define PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS 8000
uint32_t _plan_tickCount;
uint32_t _plan_startMillis;
uint32_t _plan_currentMillis;
uint32_t _plan_targetNextMessageTime;
const uint32_t LOOP_TICK_INTERVAL_MS = 1000;

void updateTrackName() {
  snprintf(trackName, sizeof(trackName), "%d_TEST_NAME", soundIndex);
}

void incrementSoundIndex() {
  soundIndex++;
  if (soundIndex > MAX_SOUND_INDEX) {
    soundIndex = MIN_SOUND_INDEX;
  }
  updateTrackName();
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
  Serial.println(F("Start the player..."));
  Serial.println(F("--------------------------------------------------------------------------------------------"));

  // Initialize trackName
  updateTrackName();

  // Initialize my player
  player.begin();

  // Set the equalizer preset
  player.setEqualizerPreset(PlayerController::EqualizerPreset::ROCK);

  // Set the volume
  player.setVolume(playerVolume);

  delay(1000);  // add a delay for clarity to see what is happening

  // Start the first sound play
  player.playSound(soundIndex, trackDurationMs, trackName);

  delay(1000);  // add a delay for clarity to see what is happening

  Serial.println(F("--------------------------------------------------------------------------------------------"));
}

void loop() {
  _plan_currentMillis = millis();

  player.update();

  // Sound playing logic
  if (!player.isSoundPlaying()) {
    incrementSoundIndex();

    Serial.printf("🔇 [%s] Not playing\n", __PRETTY_FUNCTION__);
    Serial.printf("▶️ [%s] Play sound index: %d, duration: %d ms\n", __PRETTY_FUNCTION__, soundIndex, trackDurationMs);
    player.playSound(soundIndex, trackDurationMs, trackName);
  }

  //  if (_plan_currentMillis > _plan_targetNextMessageTime) {
  //    _plan_tickCount++;
  //    _plan_targetNextMessageTime = _plan_currentMillis + PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS;
  //    Serial.printf("plan_update() :: _plan_currentMillis: %d, _plan_targetNextMessageTime: %d, _plan_tickCount: %d\n", _plan_currentMillis, _plan_targetNextMessageTime, _plan_tickCount);
  //  }  // Interval
}  // Loop
