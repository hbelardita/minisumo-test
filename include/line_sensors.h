#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H

#include <stdint.h>
#include <stdbool.h>

#define BORDER_THRESHOLD 400

struct LineSensorsReading {
    bool leftBorder;
    bool rightBorder;
    int leftRaw;
    int rightRaw;
};

/**
 * Configure TCRT5000 line sensor analog pins as INPUT.
 */
void lineSensorsInit(void);

/**
 * Check if the left Line Sensor detects the white Border (< BORDER_THRESHOLD).
 */
bool lineSensorLeftIsBorder(void);

/**
 * Check if the right Line Sensor detects the white Border (< BORDER_THRESHOLD).
 */
bool lineSensorRightIsBorder(void);

/**
 * Read both Line Sensors, returning raw ADC values and border flags.
 */
LineSensorsReading lineSensorsRead(void);

#endif // LINE_SENSORS_H
