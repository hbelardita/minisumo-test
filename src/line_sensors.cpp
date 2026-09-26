#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "line_sensors.h"

static inline bool isBorderValue(int adcValue) {
    return adcValue < BORDER_THRESHOLD;
}

void lineSensorsInit(void) {
    pinMode(PIN_TCRT_LEFT, INPUT);
    pinMode(PIN_TCRT_RIGHT, INPUT);
}

bool lineSensorLeftIsBorder(void) {
    return isBorderValue(analogRead(PIN_TCRT_LEFT));
}

bool lineSensorRightIsBorder(void) {
    return isBorderValue(analogRead(PIN_TCRT_RIGHT));
}

LineSensorsReading lineSensorsRead(void) {
    LineSensorsReading reading;
    reading.leftRaw = analogRead(PIN_TCRT_LEFT);
    reading.rightRaw = analogRead(PIN_TCRT_RIGHT);
    reading.leftBorder = isBorderValue(reading.leftRaw);
    reading.rightBorder = isBorderValue(reading.rightRaw);
    return reading;
}
