#include <SoftwareSerial.h>

// uint32_t _startMillis;
// uint32_t _currentMillis;

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
  #include "MDPlayerController.h"
  MDPlayerController player(MD_PLAYER_RX_PIN, MD_PLAYER_TX_PIN);
#elif defined(DY_PLAYER_ENABLED)
  #include "DYPlayerController.h"
  DYPlayerController player(DY_PLAYER_RX_PIN, DY_PLAYER_TX_PIN);
#elif defined(DF_PLAYER_ENABLED)
  #include "DFRobotPlayerController.h"
  DFRobotPlayerController player(DF_PLAYER_RX_PIN, DF_PLAYER_TX_PIN);
#endif

int soundIndex = 1;
uint8_t volume;
// static uint32_t lastSoundPlayedTime = 0;
// Define minutes and seconds
const uint8_t minutes = 0;
const uint8_t seconds = 20;
static const uint32_t trackDurationMs = (minutes * 60 + seconds) * 1000;

#define PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS 20000
uint32_t _plan_startMillis;
uint32_t _plan_currentMillis;
uint32_t _plan_targetNextMessageTime;
uint32_t _plan_tickCount = 0;

#ifdef RELAY_ENABLED
  #define RELAY_PIN D6
  bool relayToggle = false;
#endif

#ifdef BUTTON_ENABLED
  #define BUTTON_PIN D8
  #include <Button2.h>  // https://github.com/LennartHennigs/Button2
  Button2 button;
  unsigned long buttonPressedTimer;
  bool buttonPressedBool;
#endif

#ifdef PIR_ENABLED
  #define PIR_PIN D6
  bool pirState = LOW;  // Global variable to store the current PIR state
#endif


#ifdef FOOTSWITCH_ENABLED
  #define FOOTSWITCH_PIN D8
  #include <Button2.h>  // https://github.com/LennartHennigs/Button2
  Button2 footswitch;
  unsigned long footswitchPressedTimer;
  bool footswitchPressedBool;
#endif


int planTriggerStartMs;
int planTriggerElapsedMs;
int planTriggerTimeoutMs;
int planTriggerCount;
bool planActive = false;

const uint32_t _PLAN_TARGET_LOOP_TICK_INTERVAL_MS = 1000;

void plan_begin() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("--------------------------------------------------------------------------------------------"));
  Serial.println(__FILE__);
  Serial.println("Compiled " __DATE__ " of " __TIME__);
  Serial.println(F("--------------------------------------------------------------------------------------------"));
//   Serial.println(F("PLAN  ------------------------------------------------------------------------------------------"));
  // Serial.printf("      plan_begin %s\n", PLAN_NAME);
   Serial.println(F("PLAN SETUP ------------------------------------------------------------------------------------------"));

  #ifdef BUTTON_ENABLED
    Serial.printf("      Setup button on GPIO pin: %i (BUTTON_PIN)\n", BUTTON_PIN);
    button.begin(BUTTON_PIN);
    button.setTapHandler(tapHandler);
    Serial.println(F("PLAN SETUP ----------------------------------------------------------------------------------------"));
  #endif

  #ifdef RELAY_ENABLED
    Serial.printf("      Setup RELAY_PIN to OUTPUT\n");
    pinMode(RELAY_PIN, OUTPUT);
    Serial.println(F("PLAN SETUP ------------------------------------------------------------------------------------------"));
  #endif

  player.begin();
  player.setVolume(100);
}

void setup() {
  plan_begin();
}

void plan_trigger() {
  Serial.printf("PLAN_TRIGGER\n");
  Serial.printf("  Plan triggered #%d\n", planTriggerCount);

    player.setVolume(100);
    player.playSound(soundIndex, 20000);
}

  void plan_stop() {
    Serial.println("plan_stop()");
    player.stopSound();
}

const unsigned long LONG_PRESS_DURATION = 3000;  // 3 seconds for long press
const unsigned long LONG_PRESS_DEBOUNCE = 3000;  // 3 seconds debounce after long press
unsigned long buttonPressStartTime = 0;
unsigned long lastLongPressTime = 0;
bool buttonLongPressHandled = false;

void plan_update() {
  _plan_currentMillis = millis();  // Set current time immediately

  // generic_timer.handle();

  player.update();

  #ifdef BUTTON_ENABLED
    button.loop();
  #endif

  if (_plan_currentMillis > _plan_targetNextMessageTime) {
    _plan_tickCount++;
    _plan_targetNextMessageTime = _plan_currentMillis + _PLAN_TARGET_LOOP_TICK_INTERVAL_MS;
  }

  // Sound playing logic
  if (!player.isSoundPlaying() && planActive) {
    Serial.printf("NOT PLAYING\n");
    player.setVolume(100);
    player.playSound(soundIndex, trackDurationMs);
    delay(300); // wait a bit so we are sure that the sound has started
  }
} // plan_update()

void loop() {
   plan_update();
}
