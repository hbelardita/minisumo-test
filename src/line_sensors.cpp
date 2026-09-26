#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "line_sensors.h"

static bool s_leftBorder = false;
static bool s_rightBorder = false;

static inline bool updateBorderState(int adcValue, bool currentState) {
    if (currentState) {
        if (adcValue >= BORDER_EXIT_THRESHOLD) {
            return false;
        }
        return true;
    } else {
        if (adcValue < BORDER_THRESHOLD) {
            return true;
        }
        return false;
    }
}

void lineSensorsInit(void) {
    pinMode(PIN_TCRT_LEFT, INPUT);
    pinMode(PIN_TCRT_RIGHT, INPUT);
    s_leftBorder = false;
    s_rightBorder = false;
}

bool lineSensorLeftIsBorder(void) {
    s_leftBorder = updateBorderState(analogRead(PIN_TCRT_LEFT), s_leftBorder);
    return s_leftBorder;
}

bool lineSensorRightIsBorder(void) {
    s_rightBorder = updateBorderState(analogRead(PIN_TCRT_RIGHT), s_rightBorder);
    return s_rightBorder;
}

LineSensorsReading lineSensorsRead(void) {
    LineSensorsReading reading;
    reading.leftRaw = analogRead(PIN_TCRT_LEFT);
    reading.rightRaw = analogRead(PIN_TCRT_RIGHT);
    s_leftBorder = updateBorderState(reading.leftRaw, s_leftBorder);
    s_rightBorder = updateBorderState(reading.rightRaw, s_rightBorder);
    reading.leftBorder = s_leftBorder;
    reading.rightBorder = s_rightBorder;
    return reading;
}
