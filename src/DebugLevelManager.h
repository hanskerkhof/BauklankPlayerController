// DebugLevelManager.h
#pragma once
#include <Arduino.h>

// Debug levels as bit flags
enum class DebugLevel : uint32_t {
    NONE      = 0,
    SETUP     = 1 << 7,  // Debug setup information
    COMMANDS  = 1 << 0,  // Debug command execution
    REALTIME  = 1 << 1,  // Debug realtime loop information
    FADE      = 1 << 8,  // Debug fade operations
    NETWORK   = 1 << 2,  // Debug network operations
    FIXTURES  = 1 << 3,  // Debug fixture-related operations
    FADING    = 1 << 4,  // Debug fading operations
    PLAYBACK  = 1 << 5,  // Debug playback operations
    UPDATE    = 1 << 9,  // Debug update operations
    VOLUME    = 1 << 6,  // Debug volume changes
    ALL       = 0xFFFFFFFF
};

// Operator overloads for DebugLevel enum class
inline DebugLevel operator|(DebugLevel a, DebugLevel b) {
    return static_cast<DebugLevel>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline DebugLevel operator&(DebugLevel a, DebugLevel b) {
    return static_cast<DebugLevel>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

// Current debug level - can be changed at runtime
extern DebugLevel CURRENT_DEBUG_LEVEL;

// Debug macro that checks if a specific debug level is enabled
#define DEBUG_ENABLED(level) \
    (static_cast<uint32_t>(CURRENT_DEBUG_LEVEL) & static_cast<uint32_t>(level))

// Helper function to get combined debug level names
inline String getDebugLevelNames(DebugLevel level, const char* operation) {
    String debugLevels;
    if (level == DebugLevel::ALL) {
        debugLevels = "ALL";
    } else if (level == DebugLevel::NONE) {
        debugLevels = "NONE";
    } else {
        String separator = (strcmp(operation, "AND") == 0) ? "&" : "|";
        if ((level & DebugLevel::SETUP) != DebugLevel::NONE) debugLevels += "SETUP" + separator;
        if ((level & DebugLevel::COMMANDS) != DebugLevel::NONE) debugLevels += "COMMANDS" + separator;
        if ((level & DebugLevel::REALTIME) != DebugLevel::NONE) debugLevels += "REALTIME" + separator;
        if ((level & DebugLevel::FADE) != DebugLevel::NONE) debugLevels += "FADE" + separator;
        if ((level & DebugLevel::NETWORK) != DebugLevel::NONE) debugLevels += "NETWORK" + separator;
        if ((level & DebugLevel::FIXTURES) != DebugLevel::NONE) debugLevels += "FIXTURES" + separator;
        if ((level & DebugLevel::FADING) != DebugLevel::NONE) debugLevels += "FADING" + separator;
        if ((level & DebugLevel::PLAYBACK) != DebugLevel::NONE) debugLevels += "PLAYBACK" + separator;
        if ((level & DebugLevel::UPDATE) != DebugLevel::NONE) debugLevels += "UPDATE" + separator;
        if ((level & DebugLevel::VOLUME) != DebugLevel::NONE) debugLevels += "VOLUME" + separator;
        if (debugLevels.length() > 0) {
            debugLevels.remove(debugLevels.length() - 1); // Remove trailing separator
        }
    }
    return debugLevels;
}
//inline String getDebugLevelNames(uint32_t level, const char* operation) {
////inline String getDebugLevelNames(DebugLevel level) {
//    String debugLevels;
//    if (level == DebugLevel::ALL) {
//        debugLevels = "ALL";
//    } else if (level == DebugLevel::NONE) {
//        debugLevels = "NONE";
//    } else {
//        String separator = (strcmp(operation, "AND") == 0) ? "&" : "|";
//        if (static_cast<uint32_t>(level & DebugLevel::SETUP) != 0) debugLevels += "SETUP|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::COMMANDS) != 0) debugLevels += "COMMANDS|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::REALTIME) != 0) debugLevels += "REALTIME|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::FADE) != 0) debugLevels += "FADE|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::NETWORK) != 0) debugLevels += "NETWORK|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::FIXTURES) != 0) debugLevels += "FIXTURES|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::FADING) != 0) debugLevels += "FADING|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::PLAYBACK) != 0) debugLevels += "PLAYBACK|" + separator;
//        if (static_cast<uint32_t>(level & DebugLevel::VOLUME) != 0) debugLevels += "VOLUME|" + separator;
//        if (debugLevels.length() > 0) {
//            debugLevels.remove(debugLevels.length() - 1); // Remove trailing separator
//        }
//    }
//    return debugLevels;
//}

// Debug print macro
/**
 * @brief Debug print macro that triggers if ANY specified debug level is enabled
 * @param level One or more debug levels combined with | operator
 * @param fmt Printf-style format string
 * @param ... Variable arguments for the format string
 *
 * Prints the debug message if ANY of the specified debug levels are enabled
 * in CURRENT_DEBUG_LEVEL. Use DEBUG_PRINT_AND if you need ALL levels to be enabled.
 *
 * Example:
 *   DEBUG_PRINT(DebugLevel::REALTIME | DebugLevel::FADE, "Message %d", value);
 *   // Prints if either REALTIME or FADE level is enabled
 */
#define DEBUG_PRINT(level, fmt, ...) \
    do { \
        if (DEBUG_ENABLED(level)) { \
            String _debug_names = getDebugLevelNames(level, "OR"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__); \
        } \
    } while(0)

// Alias for explicit OR operation naming
#define DEBUG_PRINT_OR DEBUG_PRINT

/**
 * @brief Debug print macro that requires ALL specified debug levels to be enabled
 * @param level One or more debug levels combined with | operator
 * @param fmt Printf-style format string
 * @param ... Variable arguments for the format string
 *
 * Unlike DEBUG_PRINT/DEBUG_PRINT_OR which triggers if ANY specified level is enabled,
 * this macro only prints if ALL specified debug levels are enabled in CURRENT_DEBUG_LEVEL.
 *
 * Example:
 *   DEBUG_PRINT_AND(DebugLevel::REALTIME | DebugLevel::FADE, "Message %d", value);
 *   // Only prints if both REALTIME and FADE levels are enabled
 */
#define DEBUG_PRINT_AND(level, fmt, ...) \
    do { \
        if ((static_cast<uint32_t>(CURRENT_DEBUG_LEVEL) & static_cast<uint32_t>(level)) == static_cast<uint32_t>(level)) { \
            String _debug_names = getDebugLevelNames(level, "AND"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__); \
        } \
    } while(0)

//// Debug print macro
//#define DEBUG_PRINT(level, fmt, ...) \
//    do { \
//        if (DEBUG_ENABLED(level)) { \
//            Serial.printf("[DEBUG:%s] " fmt "\n", getDebugLevelName(level), ##__VA_ARGS__); \
//        } \
//    } while(0)

// Helper function to get debug level name
const char* getDebugLevelName(DebugLevel level);