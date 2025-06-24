// examples/CustomDebugLevels/CustomDebugLevels.ino

#include "DebugLevelManager2.h"

// Define your custom debug levels as an enum class
enum class MyDebugLevel : uint32_t {
    None     = 0,
    Wifi     = 1 << 0,  // Network related debugging
    Sensors  = 1 << 1,  // Sensor readings and calibration
    Screen   = 1 << 2,  // Screen updates and rendering
    Storage  = 1 << 3,  // File and EEPROM operations
    Mqtt     = 1 << 4,  // MQTT communication
    All      = 0xFFFFFFFF
};

// Define operators for the enum class
inline MyDebugLevel operator|(MyDebugLevel a, MyDebugLevel b) {
    return static_cast<MyDebugLevel>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

inline MyDebugLevel operator&(MyDebugLevel a, MyDebugLevel b) {
    return static_cast<MyDebugLevel>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

// Define debug level names (must match the enum order)
const char* debugLevelNames[] = {
    "WIFI",
    "SENSORS",
    "SCREEN",
    "STORAGE",
    "MQTT"
};

// Create debug level manager instance
DebugLevelManager<MyDebugLevel> debug(debugLevelNames, sizeof(debugLevelNames)/sizeof(debugLevelNames[0]));

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    // Set initial debug levels
    debug.setDebugLevel(MyDebugLevel::Wifi | MyDebugLevel::Sensors);

    debug.printSystemInfo();

// // Yes, you can execute functions or use conditional statements within DEBUG_PRINT by using the do-while(0) pattern that's already in the macro.
//  Here are several examples:

// // Function execution examples
// DEBUG_PRINT(debug, MyDebugLevel::Wifi, "Memory free: %d", getFreeMemory());

// // Multiple function calls
// DEBUG_PRINT(debug, MyDebugLevel::Sensors, "Temp: %.2f, Humidity: %.2f", 
//     readTemperature(), readHumidity());

// // Conditional execution with functions
// DEBUG_PRINT(debug, MyDebugLevel::Screen, 
//     isError() ? "Display Error: %s" : "Display OK: %s", 
//     getDisplayStatus());

// // Execute function only if debug level is enabled
// if (debug.isEnabled(MyDebugLevel::Storage)) {
//     DEBUG_PRINT(debug, MyDebugLevel::Storage, "Files: %d", countFiles());
//     cleanupOldFiles();  // Additional function execution
// }

// // Complex conditional with multiple functions
// DEBUG_PRINT(debug, MyDebugLevel::Mqtt, 
//     isMqttConnected() ? 
//         "MQTT OK - Topics: %d" : 
//         "MQTT Error: %s", 
//     isMqttConnected() ? 
//         countSubscribedTopics() : 
//         getMqttError());

// // Function execution with side effects
// DEBUG_PRINT(debug, MyDebugLevel::Wifi, "Reset count: %d", 
//     incrementAndGetResetCount());

//     // For functions that return values
    // DEBUG_PRINT_WITH_RETURN(debug, MyDebugLevel::Wifi, getWifiStrength(), 
    //     "WiFi strength: %d");
    
    // // For void functions
    // DEBUG_PRINT_WITH_ACTION(debug, MyDebugLevel::Screen, 
    //     updateDisplay(), 
    //     "Display updated");
        
    // // Multiple actions
    // DEBUG_PRINT_WITH_ACTION(debug, MyDebugLevel::Storage, 
    //     cleanupFiles(); 
    //     updateIndex(), 
    //     "Storage maintenance completed");
        
    // // Combining both
    // DEBUG_PRINT_WITH_ACTION(debug, MyDebugLevel::Sensors, 
    //     calibrateSensor();
    //     Serial.printf("Intermediate step\n");
    //     updateReadings(), 
    //     "Sensor calibration completed");

    DEBUG_PRINT(debug, MyDebugLevel::Wifi, "WiFi initialization started");
    DEBUG_PRINT_AND(debug, MyDebugLevel::Wifi | MyDebugLevel::Sensors,
                   "Both WiFi and Sensors debugging enabled");

    // Example of OR usage - this will print if ANY of the specified levels are enabled
    DEBUG_PRINT_OR(debug, MyDebugLevel::Wifi | MyDebugLevel::Screen,
                  "This prints if either Wifi OR Screen debugging is enabled");

    // Example of AND usage - this will only print if ALL specified levels are enabled
    DEBUG_PRINT_AND(debug, MyDebugLevel::Wifi | MyDebugLevel::Sensors,
                   "This prints only if BOTH Wifi AND Sensors debugging are enabled");

    // Single level debug print
    DEBUG_PRINT(debug, MyDebugLevel::Wifi, "WiFi initialization started");

    // Complex OR combination
    DEBUG_PRINT_OR(debug, MyDebugLevel::Storage | MyDebugLevel::Mqtt | MyDebugLevel::Screen,
                  "This prints if any of Storage, MQTT, or Screen debugging is enabled");
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 2000) {
        DEBUG_PRINT(debug, MyDebugLevel::Sensors, "Sensor update check");
        lastUpdate = millis();
    }
}