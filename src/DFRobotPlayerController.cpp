
#include "DFRobotPlayerController.h"
#include "DebugLevelManager.h"
#include <Arduino.h>

#if defined(ESP32)
    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin, int uart)
        : mySerial(uart) {
        mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
    }
#else
    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin)
        : mySoftwareSerial(rxPin, txPin) {
    }
#endif

void DFRobotPlayerController::begin() {
    // Call base class begin() first
    PlayerController::begin();

    delay(300);  // Give some time for the serial connection to establish

    Serial.println(F("  SETUP - Initializing DFPlayer ... (May take 3~5 seconds)"));
    Serial.println(F("  DFPlayer SetTimeOut(1000)"));

    myDFPlayer.setTimeOut(1000);  //Set serial communication time out to 1000ms
    delay(30);

    if (!myDFPlayer.begin(
        #if defined(ESP32)
            mySerial,
        #else
            mySoftwareSerial,
        #endif
        /*isACK = */true, /*doReset = */false)) {

        Serial.println(F("  SETUP - Unable to begin:"));
        Serial.println(F("          1.Please recheck the connection!"));
        Serial.println(F("          2.Please insert the SD card!"));

        while(true){
            delay(0); // Code to compatible with ESP32 watch dog.
        }
    }

    Serial.println(F("  SETUP - DFPlayer Mini connected."));
}

void DFRobotPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    int _folder = ((track - 1) / 255) + 1;
    int _track = ((track - 1) % 255) + 1;

    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK,
                "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms",
                __PRETTY_FUNCTION__, track, trackName, durationMs);

    myDFPlayer.playFolder(_folder, _track);

    // Call the base class for status, duration and trackName
    PlayerController::playSoundSetStatus(track, durationMs, trackName);
}

void DFRobotPlayerController::stopSound() {
    Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
    Serial.printf("      %s - myDFPlayer.stop()\n", __PRETTY_FUNCTION__);

    myDFPlayer.stop();

    // Call base class for status
    PlayerController::stopSoundSetStatus();
}

void DFRobotPlayerController::setPlayerVolume(uint8_t _playerVolume) {
    if (_playerVolume != lastSetPlayerVolume) {
        myDFPlayer.volume(_playerVolume);
        lastSetPlayerVolume = _playerVolume;
        Serial.printf("%s - Set DF Player volume to %d\n", __PRETTY_FUNCTION__, _playerVolume);
    } else {
        Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, _playerVolume);
    }
    delay(20);
}

void DFRobotPlayerController::enableLoop() {
    delay(20);
}

void DFRobotPlayerController::disableLoop() {
    delay(20);
}

void DFRobotPlayerController::setEqualizerPreset(EqualizerPreset preset) {
    switch(preset) {
        case EqualizerPreset::NORMAL:
            myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
            break;
        case EqualizerPreset::POP:
            myDFPlayer.EQ(DFPLAYER_EQ_POP);
            break;
        case EqualizerPreset::ROCK:
            myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
            break;
        case EqualizerPreset::JAZZ:
            myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
            break;
        case EqualizerPreset::CLASSIC:
            myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
            break;
        case EqualizerPreset::BASS:
            myDFPlayer.EQ(DFPLAYER_EQ_BASS);
            break;
    }
    delay(20);
    // call base class for status
    PlayerController::setEqualizerPreset(preset);
}

void DFRobotPlayerController::update() {
    PlayerController::update(); // Call the base class update method
}

//#include "DFRobotPlayerController.h"
//#include "DebugLevelManager.h"
//#include <Arduino.h>
//// MAKE SURE YOU ARE USING VERSION 1.0.5 OF THE DFRobotDFPlayerMini LIBRARY!!
//#include "DFRobotDFPlayerMini.h"
//
//
////#if defined(ESP32)
////  DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin, int uart)
////      : mySerial(uart) {
////      mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
////  }
////#else
////  DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin)
////      : mySoftwareSerial(rxPin, txPin) {
////  }
////#endif
//
//#if defined(ESP32)
//    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin, int uart)
//        : mySerial(uart) {
//        mySerial.begin(9600, SERIAL_8N1, rxPin, txPin);
//    }
//#else
//    DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin)
//        : mySoftwareSerial(rxPin, txPin) {
//    }
//#endif
//
//
//void DFRobotPlayerController::begin() {
//    // Call base class begin() first
//    PlayerController::begin();
//
//    #if defined(ESP32)
//        if (!myDFPlayer.begin(mySerial, /*isACK = */true, /*doReset = */false)) {
//    #else
//        mySoftwareSerial.begin(9600);
//        delay(300);  // Give some time for the serial connection to establish
//        if (!myDFPlayer.begin(mySoftwareSerial, /*isACK = */true, /*doReset = */false)) {
//    #endif
//
//    Serial.println(F("  SETUP - Initializing DFPlayer ... (May take 3~5 seconds)"));
//    Serial.println(F("  DFPlayer SetTimeOut(1000)"));
//
//    myDFPlayer.setTimeOut(1000);  //Set serial communication time out to 1000ms
//    delay(30);
//
//    if (!myDFPlayer.begin(
//        #if defined(ESP32)
//            mySerial,
//        #else
//            mySoftwareSerial,
//        #endif
//        /*isACK = */true, /*doReset = */false)) {
//
//        Serial.println(F("  SETUP - Unable to begin:"));
//        Serial.println(F("          1.Please recheck the connection!"));
//        Serial.println(F("          2.Please insert the SD card!"));
//
//        while(true){
//            delay(0); // Code to compatible with ESP32 watch dog.
//        }
//    }
//
//    Serial.println(F("  SETUP - DFPlayer Mini connected."));
//}
//
//void DFRobotPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
//    int _folder = ((track - 1) / 255) + 1;
//    int _track = ((track - 1) % 255) + 1;
//
//    DEBUG_PRINT(DebugLevel::COMMANDS | DebugLevel::PLAYBACK, "  ▶️ %s - track: %u (Dec) '%s', duration: %lu ms", __PRETTY_FUNCTION__, track, trackName, durationMs);
//      //// TODO Only print this when `DebugLevel::COMMANDS` is set
//      //Serial.printf("  ▶️ %s - myDFPlayer.playFolder(%d, %d)\n",  __PRETTY_FUNCTION__, _folder, _track);
//
//    myDFPlayer.playFolder(_folder, _track);
//
//    // Call the base class for status, duration and trackName
//    PlayerController::playSoundSetStatus(track, durationMs, trackName);
//}
//
//void DFRobotPlayerController::stopSound() {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
//Serial.printf("      %s - myDFPlayer.stop()\n", __PRETTY_FUNCTION__);
//
//    myDFPlayer.stop();
//
//    // Call base class for status
//    PlayerController::stopSoundSetStatus();
//}
//
//void DFRobotPlayerController::setPlayerVolume(uint8_t _playerVolume) {
//    if (_playerVolume != lastSetPlayerVolume) {
//        myDFPlayer.volume(_playerVolume);
//        lastSetPlayerVolume = _playerVolume;
//
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("%s - Set DF Player volume to %d\n", __PRETTY_FUNCTION__, _playerVolume);
//
//    } else {
//
//// TODO Only print this when `DebugLevel::COMMANDS` is set
//Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, _playerVolume);
//
//    }
//    delay(20);
//}
//
//void DFRobotPlayerController::enableLoop() {
////    loopEnabled = true;
////    myDFPlayer.enableLoop();
////    delay(20);
//}
//
//void DFRobotPlayerController::disableLoop() {
////    loopEnabled = false;
////    myDFPlayer.disableLoop();
////    delay(20);
//}
//
//void DFRobotPlayerController::setEqualizerPreset(EqualizerPreset preset) {
//    switch(preset) {
//        case EqualizerPreset::NORMAL:
//            myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
//            break;
//        case EqualizerPreset::POP:
//            myDFPlayer.EQ(DFPLAYER_EQ_POP);
//            break;
//        case EqualizerPreset::ROCK:
//            myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
//            break;
//        case EqualizerPreset::JAZZ:
//            myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
//            break;
//        case EqualizerPreset::CLASSIC:
//            myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
//            break;
//        case EqualizerPreset::BASS:
//            myDFPlayer.EQ(DFPLAYER_EQ_BASS);
//            break;
//    }
//    delay(20);
//    // call base class for status
//    PlayerController::setEqualizerPreset(preset);
//}
//
//void DFRobotPlayerController::update() {
//    PlayerController::update(); // Call the base class update method
//    // Add any DF-specific update logic here if needed
//}