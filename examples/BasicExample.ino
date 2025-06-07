// PIN MAPPING
// D1 & D2 MD_PLAYER (standard for klank fixtures)
// D3 (CLK) TM1637 DISPLAY
// D4 BUTTON
// D5 DEFAULT LED PIN #define LED_PIN D5
// // !USED BY RELAIS! // D6 PIR (set the time pot to +/- 8 seconds)
// D6 RELAIS
// D7 (DIO) TM1637 DISPLAY
// D8 ???BUTTON
// A0 Potentiometer (standard for klank fixtures)

#include <SoftwareSerial.h>

bool ANALOG_VOLUME_ENABLED = true;
#include <movingAvg.h>  // https://github.com/JChristensen/movingAvg
#define VOLUME_ANALOG_PIN A0
#define VOLUME_ANALOG_READ_INTERVAL 10
unsigned long lastAnalogReadTime = 0;
#define VOLUME_ANALOG_NUM_SAMPLES 10
#define VOLUME_ANALOG_READ_MIN 18
// #define VOLUME_ANALOG_READ_MAX 1023
#define VOLUME_ANALOG_READ_MAX 1000
#define MINIMUM_PLAYER_VOLUME 0
#define MAXIMUM_PLAYER_VOLUME 30

uint32_t _startMillis;
uint32_t _currentMillis;


uint8_t analogAvgInterval = VOLUME_ANALOG_NUM_SAMPLES; // TODO replace with VOLUME_ANALOG_READ_SAMPLES
int analogSample = 0;                         // analog input value from the potmeter  (0-1024)
//   int analogMin = 340;                          // calibration, the min and max values the potmeter is capable of.
int analogMin = 10;                          // calibration, the min and max values the potmeter is capable of.
int analogMax = 1024;
int analogAverage = 0;
int previousVolume = -1;
int previousPlayerVolume = -1;

unsigned long lastVolumeChangeTime = 0;
const unsigned long VOLUME_DEBOUNCE_DELAY = 100; // 100ms debounce time

movingAvg avgPlayerVolume(VOLUME_ANALOG_NUM_SAMPLES);  // define the moving average object

//#define RELAY_ENABLED
//#define BUTTON_ENABLED
// #define PIR_ENABLED
//#define TM1637Display_ENABLED
//#define FOOTSWITCH_ENABLED


#define MD_PLAYER_ENABLED
// #define DY_PLAYER_ENABLED
// #define DF_PLAYER_ENABLED

#include "playerController.h"

#define MD_PLAYER_RX_PIN D1
#define MD_PLAYER_TX_PIN D2

void updatePlanTimeout();

// #define DY_PLAYER_RX_PIN D5 // Green -> TX on player - Pin next to VCC
// #define DY_PLAYER_TX_PIN D6 // Blue -> RX on player - Third pin from VCC

// #define DF_PLAYER_RX_PIN D1 // Green -> TX on player - Third pin from VCC
// #define DF_PLAYER_TX_PIN D2 // Blue -> RX on player - Pin next to VCC

#ifdef MD_PLAYER_ENABLED
  #include "MDPlayerController.h"
  MDPlayerController player(MD_PLAYER_RX_PIN, MD_PLAYER_TX_PIN);
// #elif defined(DY_PLAYER_ENABLED)
//   #include "DYPlayerController.h"
//   DYPlayerController player(DY_PLAYER_RX_PIN, DY_PLAYER_TX_PIN);
// #elif defined(DF_PLAYER_ENABLED)
//   #include "DFRobotPlayerController.h"
//   DFRobotPlayerController player(DF_PLAYER_RX_PIN, DF_PLAYER_TX_PIN);
#endif

#include <AsyncTimer.h> // https://github.com/Aasim-A/AsyncTimer
AsyncTimer generic_timer;

int soundIndex = 1;
//int soundIndex = 14;
uint8_t volume;
static uint32_t lastSoundPlayedTime = 0;
// Define minutes and seconds
const uint8_t minutes = 0;
const uint8_t seconds = 20;
static const uint32_t trackDurationMs = (minutes * 60 + seconds) * 1000;

#define PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS 20000
uint32_t _plan_startMillis;
uint32_t _plan_currentMillis;
uint32_t _plan_targetNextMessageTime;
uint32_t _plan_tickCount = 0;

enum FadeDirection { FADE_OUT, FADE_IN };
static FadeDirection fadeDirection = FADE_OUT;
unsigned long lastFadeTime = 0;
const unsigned long FADE_INTERVAL = 1200;
const int fadeDurationMs = FADE_INTERVAL - (FADE_INTERVAL / 4);

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

#ifdef TM1637Display_ENABLED
  AsyncTimer t_display1;
  int t_display1_handle;
  #include <TM1637Display.h> // https://github.com/avishorp/TM1637
  #include "charset_v2.h"
  #define CLK1 D3
  #define DIO1 D7
  TM1637Display display1(CLK1, DIO1);
  // const int TM1637Display_BRIGHTNESS = 3;  // min: 0 - max: 7 (4 steps max)
  const int TM1637Display_BRIGHTNESS = 5;  // min: 0 - max: 7 (4 steps max)
#endif

int planTriggerStartMs;
int planTriggerElapsedMs;
int planTriggerTimeoutMs;
int planTriggerCount;
bool planActive = false;

#ifdef BUTTON_ENABLED
  void tapHandler(Button2& btn) {
    Serial.println("tapHandler!!!!");
    plan_trigger();
  }
#endif // BUTTON_ENABLED

//#ifdef TM1637Display_ENABLED
//  void updateTriggerDuration() {
//    int planTriggerElapsedMs = (millis() - planTriggerStartMs);
//
//    #ifdef TM1637Display_ENABLED
//      display1.showNumberDec(planTriggerElapsedMs / 1000, false);
//    #endif
//  }

  void initializeAnalogForVolume() {
    int passes = analogAvgInterval;
    Serial.printf("Fill the average\n");

    for (int i = 0; i < passes; i++) {
      analogSample = analogRead(VOLUME_ANALOG_PIN);
      int analogAverage = avgPlayerVolume.reading(map(analogSample, analogMin, analogMax, MINIMUM_PLAYER_VOLUME, MAXIMUM_PLAYER_VOLUME));
      Serial.printf("  pass: %i/%i, analogSample: %i, analogAverage:%i\n", i, passes, analogSample, analogAverage);
      delay(15);
    }
  }
  /*
  * Read the analog value from the volume potentiometer and set the volume of the player
  * Uses a moving average filter to smooth the analog reading and ensure consistent volume changes
  */

int planTimerDurationSec = 60;

void readAnalogForVolume() {
//  Serial.printf("readAnalogForVolume\n");

  analogSample = analogRead(VOLUME_ANALOG_PIN);
  if (analogSample <= analogMin) {
    analogSample = analogMin;
  }
  int tmp = avgPlayerVolume.reading(map(analogSample, analogMin, analogMax, MINIMUM_PLAYER_VOLUME, MAXIMUM_PLAYER_VOLUME));

  unsigned long currentTime = millis();

  if (tmp != previousPlayerVolume) {
    unsigned long timeSinceLastChange = currentTime - lastVolumeChangeTime;

    if (timeSinceLastChange < VOLUME_DEBOUNCE_DELAY) {
      Serial.printf("Volume change too quick! Last change: %lu ms ago\n", timeSinceLastChange);
    }

    // Check if enough time has passed since the last volume change
    if (timeSinceLastChange >= VOLUME_DEBOUNCE_DELAY) {
      Serial.printf("  Changing volume: previousPlayerVolume: %i, new volume: %i\n", previousPlayerVolume, tmp);
      // setPlayerVolumeFromAnalog(tmp, /*doNotMapVolume*/true);
      // Serial.printf(" analogSample: %i, tmp: %i\n", analogSample, tmp);

      planTimerDurationSec = mapToTimerDuration(tmp);
      updatePlanTimeout(); // Add this line

      // If has changed then show for 3 seconds...
      displayPlanTimerDurationSec();

      if(!planActive) {
        displayPlanTimerDurationSec();
      }

      previousPlayerVolume = tmp;
      lastVolumeChangeTime = currentTime; // Update the last change time
    }
  }
}

void displayPlanTimerDurationSec() {
    char buffer[5];  // Buffer to hold the string representation of the number
    if(planTimerDurationSec >= 9999) {
      displayWord("INF", display1);
    } else if(planTimerDurationSec == 0) {
      displayWord("OFF", display1);
    } else {
      display1.showNumberDec(planTimerDurationSec, false);
    }
}

const int TIMER_DURATIONS[] = {
  0,     // 0 seconds
  15,    // 0.25 minutes
  30,    // 0.5 minutes
  45,    // 0.75 minutes
  60,    // 1 minute
  90,    // 1.5 minutes
  120,   // 2 minutes
  180,   // 3 minutes
  240,   // 4 minutes
  300,   // 5 minutes
  360,   // 6 minutes
  420,   // 7 minutes
  600,   // 10 minutes
  1200,  // 20 minutes
  1800,  // 30 minutes
  3600,   // 1 hour
//  14400,  // 4 hours
//  28800,  // 8 hours
//  86400,  // 24 hours
//  604800, // 1 week
2147483647   // indefinite (maximum signed 32-bit integer)
                // Approx. 68 years, 35 days, 3 hours, 14 minutes, 7 seconds
//  9999   // indefinite
};
const int NUM_DURATIONS = sizeof(TIMER_DURATIONS) / sizeof(TIMER_DURATIONS[0]);

int mapToTimerDuration(int value) {
    int index = map(value, 0, 30, 0, NUM_DURATIONS - 1);
    return TIMER_DURATIONS[constrain(index, 0, NUM_DURATIONS - 1)];
}

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

  #ifdef TM1637Display_ENABLED
    display1.setBrightness(TM1637Display_BRIGHTNESS);
    Serial.printf("    TM1637Display set TM1637Display_BRIGHTNESS: %i\n", TM1637Display_BRIGHTNESS);
    display1.clear();
    displayWord("BOOT", display1);
    Serial.println(F("--------------------------------------------------------------------------------------------"));
  #endif

  player.begin();
  player.setVolume(100);


  if(ANALOG_VOLUME_ENABLED) {
    Serial.printf("  SETUP - ANALOG_VOLUME_ENABLED\n");
    Serial.printf("  SETUP - Setup analog pin %d (VOLUME_ANALOG_PIN)\n", VOLUME_ANALOG_PIN);
    pinMode(VOLUME_ANALOG_PIN, INPUT);
    Serial.printf("  SETUP - Setup movingAvg with interval %i\n", analogAvgInterval);
    avgPlayerVolume.begin();
    initializeAnalogForVolume();
    readAnalogForVolume();
    Serial.println(F("--------------------------------------------------------------------------------------------"));
  }
}

void setup() {
  plan_begin();
}

void plan_trigger() {
  Serial.printf("PLAN_TRIGGER\n");
  Serial.printf("  Plan triggered #%d\n", planTriggerCount);

  if (planTimerDurationSec == 0) {
    Serial.println("Plan not triggered: Timer duration is set to 0");
    return;  // Exit the function without triggering the plan
  }

if (!planActive) {
    planTriggerCount++;
    planTriggerStartMs = millis();
    planTriggerTimeoutMs = planTriggerStartMs + (planTimerDurationSec * 1000);  // Set timeout based on timer duration
    planActive = true;

    player.setVolume(100);
    player.playSound(soundIndex, 20000);

    #ifdef TM1637Display_ENABLED
      displayWord("TRIG", display1);
//      t_display1_handle = t_display1.setTimeout([]{
//      t_display1_handle = t_display1.setInterval(updateTriggerDuration, 300);
    #endif

    #ifdef RELAY_ENABLED
      digitalWrite(RELAY_PIN, HIGH);  // Set relay HIGH when plan is triggered
      Serial.println("Relay set to HIGH");
    #endif

    Serial.printf("Plan triggered with duration: %d seconds\n", planTimerDurationSec);
  } else {
    Serial.println("Plan is already active only reset the timer duration to the value set by readAnalogForVolume");
    // Reset planTimerDurationSec to the value set by readAnalogForVolume
    if (ANALOG_VOLUME_ENABLED) {
    #ifdef TM1637Display_ENABLED
      displayWord("TRIG", display1);
    #endif
      previousPlayerVolume = -1;
      readAnalogForVolume();
    }
  }

}

  void plan_stop() {
    Serial.println("plan_stop()");
    planActive = false;  // Clear the flag when a plan is stopped

    #ifdef TM1637Display_ENABLED
//      t_display1.cancel(t_display1_handle);
      displayWord("STOP", display1);
    #endif

    #ifdef RELAY_ENABLED
      digitalWrite(RELAY_PIN, LOW);  // Set relay LOW when plan is stopped
      Serial.println("Relay set to LOW");
    #endif

    // player.fadeOut(500);
    player.stopSound();
}

const unsigned long LONG_PRESS_DURATION = 3000;  // 3 seconds for long press
const unsigned long LONG_PRESS_DEBOUNCE = 3000;  // 3 seconds debounce after long press
unsigned long buttonPressStartTime = 0;
unsigned long lastLongPressTime = 0;
bool buttonLongPressHandled = false;

void plan_update() {
  _plan_currentMillis = millis();  // Set current time immediately

  generic_timer.handle();

  player.update();

  #ifdef BUTTON_ENABLED
    button.loop();
  #endif

  if (_plan_currentMillis > _plan_targetNextMessageTime) {
    _plan_tickCount++;

  if (planActive) {
      if (planTimerDurationSec >= 9999) {
        // For infinite duration, keep relay on and display "INF"
        #ifdef RELAY_ENABLED
          digitalWrite(RELAY_PIN, HIGH);
        #endif
        #ifdef TM1637Display_ENABLED
          displayWord("INF", display1);
        #endif
//        Serial.println("Debug: Infinite duration, relay on, displaying INF");
      } else {
        int remainingSeconds = (planTriggerTimeoutMs - _plan_currentMillis) / 1000;
//        Serial.printf("Debug: planActive=true, remainingSeconds=%d\n", remainingSeconds);

        if (remainingSeconds > 0) {
//          Serial.printf("Debug: Displaying remaining time: %d seconds\n", remainingSeconds);
          display1.showNumberDec(remainingSeconds, false);
        } else {
          Serial.println("Debug: Time's up! Calling plan_stop()");
          plan_stop();
        }
      }
    } else {
//      Serial.println("Debug: planActive=false, displaying timer duration");
      displayPlanTimerDurationSec();
//      Serial.printf("Debug: planTimerDurationSec=%d\n", planTimerDurationSec);
    }

    _plan_targetNextMessageTime = _plan_currentMillis + _PLAN_TARGET_LOOP_TICK_INTERVAL_MS;
//    Serial.println("Debug: End of timer display logic");
  }

//  // Periodic update logic
//  if (_plan_currentMillis > _plan_targetNextMessageTime) {
//    _plan_tickCount++;
//    _plan_targetNextMessageTime = _plan_currentMillis + PLAN_TARGET_NEXT_MESSAGE_TIME_INTERVAL_MS;
//    Serial.println(F("--------------------------------------------------------------------------------------------"));
//    Serial.printf("plan_update() :: _plan_currentMillis: %d, _plan_targetNextMessageTime: %d, _plan_tickCount: %d\n",
//                   _plan_currentMillis, _plan_targetNextMessageTime, _plan_tickCount);
//  }

  // Sound playing logic
  if (!player.isSoundPlaying() && planActive) {
    Serial.printf("NOT PLAYING\n");
    player.setVolume(100);
    player.playSound(soundIndex, trackDurationMs);
    delay(300); // wait a bit so we are sure that the sound has started
  }

  if (ANALOG_VOLUME_ENABLED) {
    if (_plan_currentMillis - lastAnalogReadTime >= VOLUME_ANALOG_READ_INTERVAL) {
      readAnalogForVolume();
      lastAnalogReadTime = _plan_currentMillis;
    }
  }

if (planActive) {  // Only check for long press when plan is active
  if (!button.isPressed()) {
    if (buttonPressStartTime == 0) {
      buttonPressStartTime = _plan_currentMillis;
//      Serial.printf("Button pressed and buttonPressStartTime set to %lu ms\n", _plan_currentMillis);
    }

    int buttonPressDuration = _plan_currentMillis - buttonPressStartTime;

//    Serial.printf("buttonPressDuration: %lu ms\n", buttonPressDuration);
//    Serial.printf("_plan_currentMillis: %lu, buttonPressStartTime: %lu, LONG_PRESS_DURATION: %lu\n",
//                  _plan_currentMillis, buttonPressStartTime, LONG_PRESS_DURATION);
//    Serial.printf("buttonLongPressHandled: %d\n", buttonLongPressHandled);

    // Check if we're within the debounce period
    if (_plan_currentMillis - lastLongPressTime < LONG_PRESS_DEBOUNCE) {
      return;  // Exit early if we're still in the debounce period
    }

    if (!buttonLongPressHandled && buttonPressDuration >= LONG_PRESS_DURATION) {
//      Serial.printf("Button press duration: %lu ms\n", _plan_currentMillis - buttonPressStartTime);
//      Serial.printf("_plan_currentMillis: %lu, buttonPressStartTime: %lu, LONG_PRESS_DURATION: %lu\n",
//                    _plan_currentMillis, buttonPressStartTime, LONG_PRESS_DURATION);
      Serial.println("Button held for 3 seconds or more, stopping plan");
      plan_stop();  // Uncomment this if you want to actually stop the plan
      buttonLongPressHandled = true;
      buttonPressStartTime = 0;
      lastLongPressTime = _plan_currentMillis;  // Set the last long press time
    }
  } else {
    // Button is not pressed, reset the start time
    buttonPressStartTime = 0;
    buttonLongPressHandled = false;
  }
}

} // plan_update()
void loop() {
   plan_update();
}

void updatePlanTimeout() {
  if (planTimerDurationSec == 0) {
    // Immediately stop the plan
    plan_stop();
    planActive = false;
    planTriggerTimeoutMs = 0;

    #ifdef RELAY_ENABLED
      digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off
    #endif

    #ifdef TM1637Display_ENABLED
      displayWord("OFF", display1);  // Display "OFF" on the display
    #endif

    Serial.println("Plan stopped immediately due to timer set to 0");
  } else if (planActive) {
    planTriggerTimeoutMs = millis() + (planTimerDurationSec * 1000);
  }
}