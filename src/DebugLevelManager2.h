// DebugLevelManager.h
#pragma once
#include <Arduino.h>

template<typename EnumType>
class DebugLevelManager {
private:
    EnumType currentDebugLevel;
    const char** debugLevelNames;
    size_t numLevels;

public:
    // Constructor taking an array of debug level names and its size
    DebugLevelManager(const char** levelNames, size_t count)
        : currentDebugLevel(static_cast<EnumType>(0))
        , debugLevelNames(levelNames)
        , numLevels(count) {
        Serial.println(F("Debug system initialized"));
    }

    // Set current debug level
    void setDebugLevel(EnumType level) {
        currentDebugLevel = level;
    }

    // Get current debug level
    EnumType getDebugLevel() const {
        return currentDebugLevel;
    }

    // Check if a specific debug level is enabled
    bool isEnabled(EnumType level) const {
        return (static_cast<typename std::underlying_type<EnumType>::type>(currentDebugLevel) &
                static_cast<typename std::underlying_type<EnumType>::type>(level)) != 0;
    }

    // Check if ALL specified debug levels are enabled
    bool areAllEnabled(EnumType levels) const {
        return (static_cast<typename std::underlying_type<EnumType>::type>(currentDebugLevel) &
                static_cast<typename std::underlying_type<EnumType>::type>(levels))
               == static_cast<typename std::underlying_type<EnumType>::type>(levels);
    }


    String getCombinedLevelNames(EnumType level, const char* operation = "|") const {
        auto levelValue = static_cast<typename std::underlying_type<EnumType>::type>(level);

        if (levelValue == static_cast<typename std::underlying_type<EnumType>::type>(-1)) {
            return F("ALL");
        }
        if (levelValue == 0) {
            return F("NONE");
        }

        String result;
        for (size_t i = 0; i < numLevels; i++) {
            if (levelValue & (1UL << i)) {
                // Debug print
                //  Serial.print("Adding level: ");
                //  Serial.println(debugLevelNames[i]);
                //  Serial.print("Current result: ");
                //  Serial.println(result);

                if (result.length() > 0) {
                    result += String(operation);  // Convert char* to String explicitly
                }
                result += debugLevelNames[i];
            }
        }
        //  Serial.print("Final result: ");
        //  Serial.println(result);
        return result;
    }

    // Print debug system information
    void printSystemInfo() const {
        Serial.println(F("Debug System Information:"));
        Serial.println(F("------------------------"));
        Serial.println(F("Library version: 1.0.0"));
        Serial.println(F("Build Date: " __DATE__));
        Serial.println(F("Build Time: " __TIME__));
        Serial.print(F("Current Debug Level: "));
        Serial.println(getCombinedLevelNames(currentDebugLevel, "|"));
    }
};

// Debug print macros
#define DEBUG_PRINT(manager, level, fmt, ...) \
    do { \
        if (manager.isEnabled(level)) { \
            String _debug_names = manager.getCombinedLevelNames(level, "|"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_PRINT_OR DEBUG_PRINT

#define DEBUG_PRINT_AND(manager, level, fmt, ...) \
    do { \
        if (manager.areAllEnabled(level)) { \
            String _debug_names = manager.getCombinedLevelNames(level, "&"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__); \
        } \
    } while(0)

// For functions that return values
#define DEBUG_PRINT_WITH_RETURN(manager, level, func, fmt, ...) \
    do { \
        if (manager.isEnabled(level)) { \
            auto result = func; \
            String _debug_names = manager.getCombinedLevelNames(level, "|"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__, result); \
        } \
    } while(0)

// For void functions
#define DEBUG_PRINT_WITH_ACTION(manager, level, action, fmt, ...) \
    do { \
        if (manager.isEnabled(level)) { \
            action; \
            String _debug_names = manager.getCombinedLevelNames(level, "|"); \
            Serial.printf("[DEBUG:%s] " fmt "\n", _debug_names.c_str(), ##__VA_ARGS__); \
        } \
    } while(0)

// ensure newline....