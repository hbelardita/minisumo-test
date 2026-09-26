#include "mock_arduino.h"
#include "pins.h"
#include "motors.h"
#include <cassert>
#include <iostream>

ArduinoMockState mockState;

void test_pin_definitions() {
    assert(PIN_STBY == 8);
    assert(PIN_AIN1 == 7);
    assert(PIN_AIN2 == 9);
    assert(PIN_PWMA == 5);
    assert(PIN_BIN1 == 4);
    assert(PIN_BIN2 == 2);
    assert(PIN_PWMB == 6);
    std::cout << "[PASS] test_pin_definitions\n";
}

void test_motors_init() {
    mockState.reset();
    motorsInit();

    assert(mockState.pinModes[PIN_STBY] == OUTPUT);
    assert(mockState.pinModes[PIN_AIN1] == OUTPUT);
    assert(mockState.pinModes[PIN_AIN2] == OUTPUT);
    assert(mockState.pinModes[PIN_PWMA] == OUTPUT);
    assert(mockState.pinModes[PIN_BIN1] == OUTPUT);
    assert(mockState.pinModes[PIN_BIN2] == OUTPUT);
    assert(mockState.pinModes[PIN_PWMB] == OUTPUT);

    // Initial state must be coast
    assert(mockState.digitalPins[PIN_STBY] == LOW);
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 0);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMB] == 0);
    std::cout << "[PASS] test_motors_init\n";
}

void test_motors_forward() {
    mockState.reset();
    motorsInit();

    // Forward: positive speeds
    motorsSetSpeed(255, 255);

    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Channel A (Right motor): forward -> AIN1=LOW, AIN2=HIGH
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == HIGH);
    assert(mockState.analogPins[PIN_PWMA] == 255);

    // Channel B (Left motor): forward -> BIN1=LOW, BIN2=HIGH
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);
    assert(mockState.analogPins[PIN_PWMB] == 255);
    std::cout << "[PASS] test_motors_forward\n";
}

void test_motors_reverse() {
    mockState.reset();
    motorsInit();

    // Reverse: negative speeds
    motorsSetSpeed(-255, -255);

    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Channel A (Right motor): reverse -> AIN1=HIGH, AIN2=LOW
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 255);

    // Channel B (Left motor): reverse -> BIN1=HIGH, BIN2=LOW
    assert(mockState.digitalPins[PIN_BIN1] == HIGH);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMB] == 255);
    std::cout << "[PASS] test_motors_reverse\n";
}

void test_motors_zero_speed_coasts() {
    mockState.reset();
    motorsInit();

    motorsSetSpeed(200, 200);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);

    // Setting speed to (0, 0) must coast, not brake
    motorsSetSpeed(0, 0);

    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 0);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMB] == 0);
    std::cout << "[PASS] test_motors_zero_speed_coasts\n";
}

void test_motors_coast_explicit() {
    mockState.reset();
    motorsInit();

    motorsSetSpeed(255, 255);
    motorsCoast();

    assert(mockState.digitalPins[PIN_STBY] == LOW);
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 0);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMB] == 0);
    std::cout << "[PASS] test_motors_coast_explicit\n";
}

void test_motors_brake_active() {
    mockState.reset();
    motorsInit();

    motorsSetSpeed(200, 200);
    motorsBrake();

    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Short brake: both direction inputs HIGH
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == HIGH);
    assert(mockState.digitalPins[PIN_BIN1] == HIGH);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);
    std::cout << "[PASS] test_motors_brake_active\n";
}

void test_motors_clamping() {
    mockState.reset();
    motorsInit();

    motorsSetSpeed(500, -500);
    assert(mockState.analogPins[PIN_PWMA] == 255);
    assert(mockState.analogPins[PIN_PWMB] == 255);
    std::cout << "[PASS] test_motors_clamping\n";
}

int main() {
    std::cout << "--- Running Motor Driver Unit Tests ---\n";
    test_pin_definitions();
    test_motors_init();
    test_motors_forward();
    test_motors_reverse();
    test_motors_zero_speed_coasts();
    test_motors_coast_explicit();
    test_motors_brake_active();
    test_motors_clamping();
    std::cout << "All motor driver tests passed successfully!\n";
    return 0;
}
