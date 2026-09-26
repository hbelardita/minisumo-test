#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "line_sensors.h"

void lineSensorsInit(void) {
    pinMode(PIN_TCRT_LEFT, INPUT);
    pinMode(PIN_TCRT_RIGHT, INPUT);
}

bool lineSensorLeftIsBorder(void) {
    return analogRead(PIN_TCRT_LEFT) < BORDER_THRESHOLD;
}

bool lineSensorRightIsBorder(void) {
    return analogRead(PIN_TCRT_RIGHT) < BORDER_THRESHOLD;
}

LineSensorsReading lineSensorsRead(void) {
    LineSensorsReading reading;
    reading.leftRaw = analogRead(PIN_TCRT_LEFT);
    reading.rightRaw = analogRead(PIN_TCRT_RIGHT);
    reading.leftBorder = (reading.leftRaw < BORDER_THRESHOLD);
    reading.rightBorder = (reading.rightRaw < BORDER_THRESHOLD);
    return reading;
}
