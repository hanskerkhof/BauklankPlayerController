// DebugLevelManager.cpp
#include "DebugLevelManager.h"

// Initialize the current debug level
DebugLevel CURRENT_DEBUG_LEVEL = DebugLevel::NONE;

const char* getDebugLevelName(DebugLevel level) {
    switch (level) {
        case DebugLevel::NONE:     return "NONE";
        case DebugLevel::SETUP:    return "SETUP";
        case DebugLevel::COMMANDS: return "COMMANDS";
        case DebugLevel::REALTIME: return "REALTIME";
        case DebugLevel::NETWORK:  return "NETWORK";
        case DebugLevel::FIXTURES: return "FIXTURES";
        case DebugLevel::FADING:   return "FADING";
        case DebugLevel::PLAYBACK: return "PLAYBACK";
        case DebugLevel::VOLUME:   return "VOLUME";
        case DebugLevel::ALL:      return "ALL";
        default:                   return "UNKNOWN";
    }
}

