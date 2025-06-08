#ifndef ANALOG_READER_H
#define ANALOG_READER_H

#include <Arduino.h>
#include <functional>
#include <movingAvg.h>  // Include the movingAvg library


/**
 * @brief AnalogReader constructor
 *
 * @param pin The analog pin to read from
 * @param minValue The minimum value of the mapped range
 * @param maxValue The maximum value of the mapped range
 * @param readInterval The interval between reads in milliseconds
 * @param numSamples The number of samples for the moving average
 * @param debounceInterval The debounce interval in milliseconds
 *
 * Note on debounceInterval:
 * 1. For speed (faster response, may have some noise):
 *    - Use a shorter debounceInterval, e.g., 1-5 ms
 *    - This allows for quicker updates but may be more susceptible to noise
 *    - Ideal for applications where immediate response is crucial
 *
 * 2. For reliability (stable readings, slower response):
 *    - Use a longer debounceInterval, e.g., 20-50 ms
 *    - This provides more stable readings by filtering out rapid fluctuations
 *    - Ideal for applications where accuracy is more important than speed
 *
* 3. Interaction:
 *    - If debounceInterval <= readInterval: Minimal to no debouncing effect
 *    - If debounceInterval > readInterval: Helps stabilize the output
 *    - The moving average smooths the input, while debounce stabilizes the output
 *
 * Adjust these values based on your specific needs:
 * - Larger numSamples and readInterval: More smoothing, slower response
 * - Smaller numSamples and readInterval: Less smoothing, faster response
 * - Larger debounceInterval: More stable output, but slower to change
 * - Smaller debounceInterval: Quicker output changes, but may be less stable
 */
class AnalogReader {
public:
  AnalogReader(
    uint8_t pin,
    int minValue,
    int maxValue,
    unsigned long readInterval = 3,
    int numSamples = 10,
    unsigned long debounceInterval = 5)
    : _pin(pin),
      _minValue(minValue),
      _maxValue(maxValue),
      _readInterval(readInterval),
      _avgCalculator(numSamples),
      _debounceInterval(debounceInterval),
      _numSamples(numSamples)  // Initialize _numSamples
  {
    pinMode(_pin, INPUT);
    _avgCalculator.begin();
  }

  void begin() {
    Serial.println("AnalogReader: Initializing...");

    // Fill the moving average with multiple readings
    for (int i = 0; i < _numSamples; i++) {
      int rawValue = analogRead(_pin);
      _avgCalculator.reading(rawValue);
      Serial.printf("AnalogReader: Filling average, iteration %d/%d, raw value: %d\n", i + 1, _numSamples, rawValue);
      delay(1);  // Short delay to allow for slight variations in readings
    }

    int averageRaw = _avgCalculator.getAvg();
    int mappedValue = map(averageRaw, 0, 1023, _minValue, _maxValue);
    _lastMappedValue = mappedValue;

    Serial.printf("AnalogReader: Initialization complete. Final average: %d (raw), mapped to: %d\n", averageRaw, mappedValue);
    // Call the callback with the initial mapped value
    if (_callback) {
      _callback(mappedValue);
    }
  }

  //  void update() {
  //    unsigned long currentTime = millis();
  //    if (currentTime - _lastReadTime >= _readInterval) {
  //      _lastReadTime = currentTime;
  //      int rawValue = analogRead(_pin);
  //      int averageRaw = _avgCalculator.reading(rawValue);
  //      int mappedValue = map(averageRaw, 0, 1023, _minValue, _maxValue);
  //
  //      if (mappedValue != _lastMappedValue) {
  //        unsigned long timeSinceLastChange = currentTime - _lastChangeTime;
  //        if (timeSinceLastChange >= _debounceInterval) {
  //          _lastMappedValue = mappedValue;
  //          _lastChangeTime = currentTime;
  //          if (_callback) {
  //            _callback(mappedValue);
  //          }
  //        } else {
  //          _quickChangeCounter++;
  //          if (currentTime - _lastDebugPrintTime >= 1000) {
  //            Serial.printf("Analog value changing too quickly! %d quick changes in the last second. Last change: %lu ms ago\n",
  //                          _quickChangeCounter, timeSinceLastChange);
  //            _lastDebugPrintTime = currentTime;
  //            _quickChangeCounter = 0;
  //          }
  //        }
  //      }
  //    }
  //  }
  void update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastReadTime >= _readInterval) {
      _lastReadTime = currentTime;
      int rawValue = analogRead(_pin);
      int averageRaw = _avgCalculator.reading(rawValue);
      int mappedValue = map(averageRaw, 0, 1023, _minValue, _maxValue);

      if (mappedValue != _lastMappedValue) {
        unsigned long timeSinceLastChange = currentTime - _lastChangeTime;
        if (timeSinceLastChange >= _debounceInterval) {
          // Value has changed and debounce period has passed
          _lastMappedValue = mappedValue;
          _lastChangeTime = currentTime;
          if (_callback) {
            _callback(mappedValue);
          }
          _pendingCallback = false;  // [NEW] Reset the pending callback flag
        } else {
          // Value has changed but within debounce period
          _quickChangeCounter++;
          _pendingCallback = true;  // [NEW] Set the pending callback flag
          if (currentTime - _lastDebugPrintTime >= 1000) {
            Serial.printf("Analog value changing too quickly! %d quick changes in the last second. Last change: %lu ms ago\n",
                          _quickChangeCounter, timeSinceLastChange);
            _lastDebugPrintTime = currentTime;
            _quickChangeCounter = 0;
          }
        }
      } else if (_pendingCallback && (currentTime - _lastChangeTime >= _debounceInterval)) {
        // [NEW] Debounce period has passed since the last change, and we have a pending callback
        if (_callback) {
          _callback(mappedValue);
        }
        _pendingCallback = false;  // [NEW] Reset the pending callback flag after calling
      }
    }
  }
    void setCallback(std::function<void(int)> callback) {
    _callback = callback;
  }

  int getCurrentValue() const {
    return _lastMappedValue;
  }

private:
  uint8_t _pin;
  int _minValue;
  int _maxValue;
  unsigned long _readInterval;
  unsigned long _lastReadTime = 0;
  int _lastMappedValue = -1;
  std::function<void(int)> _callback;
  movingAvg _avgCalculator;
  unsigned long _debounceInterval;
  unsigned long _lastChangeTime = 0;
  unsigned long _lastDebugPrintTime = 0;
  int _quickChangeCounter = 0;
  int _numSamples;  // Add this line to declare _numSamples
  bool _pendingCallback = false; // [NEW] Flag to track if a callback is pending after debounce
};

#endif  // ANALOG_READER_H
