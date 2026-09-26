#include "mock_arduino.h"
#include "pins.h"
#include "line_sensors.h"
#include <cassert>
#include <iostream>

ArduinoMockState mockState;

void test_line_sensors_init() {
    mockState.reset();
    lineSensorsInit();

    assert(mockState.pinModes[PIN_TCRT_LEFT] == INPUT);
    assert(mockState.pinModes[PIN_TCRT_RIGHT] == INPUT);
    std::cout << "[PASS] test_line_sensors_init\n";
}

void test_line_sensors_dohyo_black_surface() {
    mockState.reset();
    lineSensorsInit();

    // Black Dohyo surface typically gives high readings (>= 400)
    mockState.analogReadValues[PIN_TCRT_LEFT] = 850;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 720;

    LineSensorsReading reading = lineSensorsRead();
    assert(reading.leftBorder == false);
    assert(reading.rightBorder == false);
    assert(reading.leftRaw == 850);
    assert(reading.rightRaw == 720);
    assert(lineSensorLeftIsBorder() == false);
    assert(lineSensorRightIsBorder() == false);
    std::cout << "[PASS] test_line_sensors_dohyo_black_surface\n";
}

void test_line_sensors_white_border_threshold() {
    mockState.reset();
    lineSensorsInit();

    // White Border reflects and gives low readings (< 400)
    mockState.analogReadValues[PIN_TCRT_LEFT] = 150;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 399;

    LineSensorsReading reading = lineSensorsRead();
    assert(reading.leftBorder == true);
    assert(reading.rightBorder == true);
    assert(lineSensorLeftIsBorder() == true);
    assert(lineSensorRightIsBorder() == true);

    // Exact threshold boundary: 400 is not border (< 400 is border)
    mockState.analogReadValues[PIN_TCRT_LEFT] = 400;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 400;
    assert(lineSensorLeftIsBorder() == false);
    assert(lineSensorRightIsBorder() == false);
    std::cout << "[PASS] test_line_sensors_white_border_threshold\n";
}

void test_line_sensors_independent_readings() {
    mockState.reset();
    lineSensorsInit();

    // Left on border, right on Dohyo
    mockState.analogReadValues[PIN_TCRT_LEFT] = 220;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 600;

    LineSensorsReading reading = lineSensorsRead();
    assert(reading.leftBorder == true);
    assert(reading.rightBorder == false);

    // Left on Dohyo, right on border
    mockState.analogReadValues[PIN_TCRT_LEFT] = 550;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 180;

    reading = lineSensorsRead();
    assert(reading.leftBorder == false);
    assert(reading.rightBorder == true);
    std::cout << "[PASS] test_line_sensors_independent_readings\n";
}

int main() {
    std::cout << "--- Running Line Sensors Unit Tests ---\n";
    test_line_sensors_init();
    test_line_sensors_dohyo_black_surface();
    test_line_sensors_white_border_threshold();
    test_line_sensors_independent_readings();
    std::cout << "All line sensor tests passed successfully!\n";
    return 0;
}
