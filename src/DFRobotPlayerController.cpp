#include "DFRobotPlayerController.h"
// MAKE SURE YOU ARE USING VERSION 1.0.5 OF THE DFRobotDFPlayerMini LIBRARY!!
#include "DFRobotDFPlayerMini.h"
#include <Arduino.h>

DFRobotPlayerController::DFRobotPlayerController(int rxPin, int txPin)
    : mySoftwareSerial(rxPin, txPin) {
    // Select TF card in the constructor???
}

void DFRobotPlayerController::begin() {
    mySoftwareSerial.begin(9600);
    delay(300);  // Give some time for the serial connection to establish

    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.println(F("  SETUP - DFRobot DFPlayer Mini"));
        Serial.println(F("  SETUP - Initializing DFPlayer ... (May take 3~5 seconds)"));
        // This comment saved me!!!
        // https://github.com/DFRobot/DFRobotDFPlayerMini/issues/9#issuecomment-514548776
        Serial.println(F("  DFPlayer SetTimeOut(1000)"));
    #endif
    myDFPlayer.setTimeOut(1000);  //Set serial communication time out to 1000ms
    delay(30);

    // if (!myDFPlayer.begin(mySoftwareSerial, /*isACK = */true, /*doReset = */false)) {  //Use serial to communicate with mp3.
//    if (!myDFPlayer.begin(mySoftwareSerial, /*isACK = */false, /*doReset = */true)) {  //Use serial to communicate with mp3.
    if (!myDFPlayer.begin(mySoftwareSerial, /*isACK = */true, /*doReset = */false)) {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.println(F("  SETUP - Unable to begin:"));
            Serial.println(F("          1.Please recheck the connection!"));
            Serial.println(F("          2.Please insert the SD card!"));
        #endif
        while(true){
            delay(0); // Code to compatible with ESP8266 watch dog.
          }
        }
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
            Serial.println(F("  SETUP - DFPlayer Mini connected."));
        #endif
    }

void DFRobotPlayerController::playSound(int track) {
    int _folder = ((track - 1) / 255) + 1;
    int _track = ((track - 1) % 255) + 1;
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ▶️ %s - myDFPlayer.playFolder(%d, %d)\n",  __PRETTY_FUNCTION__, _folder, _track);
    #endif
    myDFPlayer.playFolder(_folder, _track);
    //  delay(20);

    // Call base class for status
    PlayerController::playSound(track);
}

void DFRobotPlayerController::playSound(int track, unsigned long durationMs) {
    // Call base class for status and duration
    PlayerController::playSound(track, durationMs);

    playSound(track);
}

void DFRobotPlayerController::playSound(int track, unsigned long durationMs, const char* trackName) {
    // Call the base class for status, duration and trackName
    PlayerController::playSound(track, durationMs, trackName);

    // Then call playsound with the 2 parameters
    // playSound(track, durationMs);
    playSound(track);
}

void DFRobotPlayerController::stopSound() {
    #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
        Serial.printf("  ⏹️ %s - Stopping sound\n", __PRETTY_FUNCTION__);
    #endif
    myDFPlayer.stop();

    // Call base class for status
    PlayerController::stopSound();
}

void DFRobotPlayerController::setPlayerVolume(uint8_t _playerVolume) {
    if (_playerVolume != lastSetPlayerVolume) {
        myDFPlayer.volume(_playerVolume);
        lastSetPlayerVolume = _playerVolume;
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
//        Serial.printf("%s - Set DF Player volume to %d\n", __PRETTY_FUNCTION__, _playerVolume);
        #endif
    } else {
        #if BAUKLANK_PLAYER_CONTROLLER_DEBUG == true
//         Serial.printf("%s - Volume already set to %d\n", __PRETTY_FUNCTION__, _playerVolume);
        #endif
    }
    delay(20);
}

void DFRobotPlayerController::enableLoop() {
    loopEnabled = true;
    myDFPlayer.enableLoop();
    delay(20);
}

void DFRobotPlayerController::disableLoop() {
    loopEnabled = false;
    myDFPlayer.disableLoop();
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
    // Implement any necessary update logic here
    // For example, you might want to check if a track has finished playing
}