#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <map>
#include <vector>

#define HIGH 1
#define LOW  0

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define A6 20
#define A7 21

struct ArduinoMockState {
    std::map<uint8_t, uint8_t> pinModes;
    std::map<uint8_t, uint8_t> digitalPins;
    std::map<uint8_t, int> analogPins;
    std::map<uint8_t, int> analogReadValues;
    unsigned long currentMillis = 0;
    unsigned long currentMicros = 0;

    void reset() {
        pinModes.clear();
        digitalPins.clear();
        analogPins.clear();
        analogReadValues.clear();
        currentMillis = 0;
        currentMicros = 0;
    }
};

extern ArduinoMockState mockState;

#ifndef __AVR__
inline uint8_t PCICR = 0;
inline uint8_t PCMSK0 = 0;
#define PCIE0 0
#define PCINT4 4
#define ISR(vector) void vector(void)
#endif

inline void noInterrupts() {}
inline void interrupts() {}

inline void pinMode(uint8_t pin, uint8_t mode) {
    mockState.pinModes[pin] = mode;
}

inline void digitalWrite(uint8_t pin, uint8_t val) {
    mockState.digitalPins[pin] = val;
}

inline int digitalRead(uint8_t pin) {
    return mockState.digitalPins[pin];
}

inline void analogWrite(uint8_t pin, int val) {
    mockState.analogPins[pin] = val;
}

inline int analogRead(uint8_t pin) {
    return mockState.analogReadValues[pin];
}

inline unsigned long millis() {
    return mockState.currentMillis;
}

inline unsigned long micros() {
    return mockState.currentMicros;
}

inline void delayMicroseconds(unsigned int us) {
    mockState.currentMicros += us;
}

#endif // MOCK_ARDUINO_H
