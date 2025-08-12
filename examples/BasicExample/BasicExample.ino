#include "AnalogReader.h"

#define NO_PLAYER_ENABLED
// #define MD_PLAYER_ENABLED
// #define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED

#include <BauklankPlayerController.h>

#define NO_PLAYER_RX_PIN 0
#define NO_PLAYER_TX_PIN 2

// #define MD_PLAYER_RX_PIN D1
// #define MD_PLAYER_TX_PIN D2

// #define DY_PLAYER_RX_PIN D5 // Green -> TX on player - Pin next to VCC
// #define DY_PLAYER_TX_PIN D6 // Blue -> RX on player - Third pin from VCC

// #define DF_PLAYER_RX_PIN D1 // Green -> TX on player - Third pin from VCC
// #define DF_PLAYER_TX_PIN D2 // Blue -> RX on player - Pin next to VCC

#ifdef MD_PLAYER_ENABLED
  #include <MDPlayerController.h>
  MDPlayerController player(MD_PLAYER_RX_PIN, MD_PLAYER_TX_PIN);
#elif defined(DY_PLAYER_ENABLED)
  #include <DYPlayerController.h>
  DYPlayerController player(DY_PLAYER_RX_PIN, DY_PLAYER_TX_PIN);
#elif defined(DF_PLAYER_ENABLED)
  #include <DFRobotPlayerController.h>
  DFRobotPlayerController player(DF_PLAYER_RX_PIN, DF_PLAYER_TX_PIN);
#elif defined(NO_PLAYER_ENABLED)
  #include <NOPlayerController.h>
  NOPlayerController player(NO_PLAYER_RX_PIN, NO_PLAYER_TX_PIN);
#endif


#define MAX_SOUND_INDEX 14
int soundIndex = 1;
uint8_t playerVolume;

// static uint32_t lastSoundPlayedTime = 0;
// Define minutes and seconds
const uint8_t minutes = 0;
const uint8_t seconds = 3;
static const uint32_t trackDurationMs = (minutes * 60 + seconds) * 1000;

int VOLUME_ANALOG_NUM_SAMPLES = 10;
int initialVolume = 19;
AnalogReader analogReader(
  /*pin=*/A0,
  /*minValue=*/0,
  /*maxValue=*/30,
  /*readInterval=*/3,
  /*numSamples=*/VOLUME_ANALOG_NUM_SAMPLES,
  /*debounceInterval=*/50 // Must be greater than 40 to work because the player.setVolume command takes a minimum of 40ms
);

void onAnalogValueChanged(int newValue) {
  Serial.printf("🔊 [%s] Analog value changed to: %d\n", __PRETTY_FUNCTION__, newValue);
  playerVolume = newValue;
  player.setVolume(playerVolume);
  // Add your logic here to handle the new value
  // Here you can update your volume or perform any other action
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
}

void loop() {
  player.update();
#ifndef NO_PLAYER_ENABLED
  analogReader.update();

  // Sound playing logic
  if (!player.isSoundPlaying()) {
    Serial.printf("🔇 [%s] Not playing\n", __PRETTY_FUNCTION__);
    Serial.printf("🔊 [%s] Setting volume to: %d\n", __PRETTY_FUNCTION__, playerVolume);
    player.setVolume(playerVolume);
    Serial.printf("▶️ [%s] Playing sound index: %d, duration: %d ms\n", __PRETTY_FUNCTION__, soundIndex, trackDurationMs);
    player.playSound(soundIndex, trackDurationMs, "TEST_SOUND");

    // Increment soundIndex and reset if it exceeds MAX_SOUND_INDEX
    soundIndex++;
    if (soundIndex > MAX_SOUND_INDEX) {
      soundIndex = 1;
    }

    delay(20); // wait a bit so we are sure that the sound has started
  }
#endif
}
