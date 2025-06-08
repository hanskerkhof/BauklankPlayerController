#include <SoftwareSerial.h>

#define MD_PLAYER_ENABLED
// #define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED

#include <BauklankPlayerController.h>

#define MD_PLAYER_RX_PIN D1
#define MD_PLAYER_TX_PIN D2

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
#endif

int soundIndex = 1;
uint8_t volume = 100;
// static uint32_t lastSoundPlayedTime = 0;
// Define minutes and seconds
const uint8_t minutes = 0;
const uint8_t seconds = 20;
static const uint32_t trackDurationMs = (minutes * 60 + seconds) * 1000;

#define PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS 20000
uint32_t _plan_startMillis;
uint32_t _plan_currentMillis;
uint32_t _plan_targetNextMessageTime;

const uint32_t LOOP_TICK_INTERVAL_MS = 1000;

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
  player.setVolume(volume);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
}

void loop() {
  player.update();

  // Sound playing logic
  if (!player.isSoundPlaying()) {
    Serial.printf("NOT PLAYING\n");
    player.setVolume(volume);
    player.playSound(soundIndex, trackDurationMs);
    delay(20); // wait a bit so we are sure that the sound has started
  }
}
