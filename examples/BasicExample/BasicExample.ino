// #define NO_PLAYER_ENABLED
// #define MD_PLAYER_ENABLED
// #define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED
// #define AK_PLAYER_ENABLED
#define XY_PLAYER_ENABLED

#include <BauklankPlayerController.h>

#define NO_PLAYER_RX_PIN 0
#define NO_PLAYER_TX_PIN 2

// #define MD_PLAYER_RX_PIN D1
// #define MD_PLAYER_TX_PIN D2
#define MD_PLAYER_RX_PIN 17 // uart2
#define MD_PLAYER_TX_PIN 18 // uart2

// #define DY_PLAYER_RX_PIN D5 // Green -> TX on player - Pin next to VCC
// #define DY_PLAYER_TX_PIN D6 // Blue -> RX on player - Third pin from VCC

// #define DF_PLAYER_RX_PIN D1 // Green -> TX on player - Third pin from VCC
// #define DF_PLAYER_TX_PIN D2 // Blue -> RX on player - Pin next to VCC

#define XY_PLAYER_RX_PIN D1 // Green -> TX on player - ???
#define XY_PLAYER_TX_PIN D2 // Blue -> RX on player - ???

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
#elif defined(AK_PLAYER_ENABLED)
  #include <AKPlayerController.h>
  AKPlayerController player;
#elif defined(XY_PLAYER_ENABLED)
  #include <XYPlayerController.h>
  XYPlayerController player(XY_PLAYER_RX_PIN, XY_PLAYER_TX_PIN);
#endif

int minSoundIndex = 200;
int maxSoundIndex = 213;
int currentSoundIndex = minSoundIndex;
uint8_t playerVolume = 25; // 0-30

// Define minutes and seconds
const uint8_t minutes = 0;
const uint8_t seconds = 10;
static const uint32_t trackDurationMs = (minutes * 60 + seconds) * 1000;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  Serial.println(__FILE__);
  Serial.println("Compiled " __DATE__ " of " __TIME__);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  player.begin();
  Serial.println(F("--------------------------------------------------------------------------------------------"));
}

void loop() {
  player.update();
  #ifndef NO_PLAYER_ENABLED
    // Sound playing logic
    if (!player.isSoundPlaying()) {
      Serial.printf("🔇 [%s] Not playing\n", __PRETTY_FUNCTION__);
      // Serial.printf("🔊 [%s] Setting volume to: %d\n", __PRETTY_FUNCTION__, playerVolume);
      // player.setVolume(playerVolume);
      Serial.printf("▶️ [%s] Playing sound currentSoundIndex: %d, duration: %d ms\n", __PRETTY_FUNCTION__, currentSoundIndex, trackDurationMs);
      player.playTrack(currentSoundIndex, trackDurationMs, "TEST_SOUND");

      // Increment soundIndex and reset if it exceeds maxSoundIndex
      currentSoundIndex++;
      if (currentSoundIndex > maxSoundIndex) {
        currentSoundIndex = minSoundIndex;
      }

      delay(20); // wait a bit so we are sure that the sound has started
    }
  #endif
}
