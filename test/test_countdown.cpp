#include "mock_arduino.h"
#include "pins.h"
#include "countdown.h"
#include <cassert>
#include <iostream>

ArduinoMockState mockState;

void test_countdown_init() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH; // Unpressed with pullup
    countdownInit();

    assert(mockState.pinModes[PIN_BUTTON] == INPUT_PULLUP);
    assert(mockState.pinModes[PIN_LED] == OUTPUT);
    assert(mockState.digitalPins[PIN_LED] == LOW);
    assert(countdownGetState() == COUNTDOWN_IDLE);
    assert(countdownIsRunning() == false);
    assert(countdownIsComplete() == false);
    std::cout << "[PASS] test_countdown_init\n";
}

void test_countdown_button_trigger() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    countdownInit();

    // Button pressed (active LOW)
    mockState.digitalPins[PIN_BUTTON] = LOW;
    countdownUpdate();

    assert(countdownGetState() == COUNTDOWN_RUNNING);
    assert(countdownIsRunning() == true);
    assert(countdownIsComplete() == false);
    assert(mockState.digitalPins[PIN_LED] == HIGH);
    std::cout << "[PASS] test_countdown_button_trigger\n";
}

void test_countdown_manual_start() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    countdownInit();

    countdownStart();
    assert(countdownIsRunning() == true);
    assert(countdownGetState() == COUNTDOWN_RUNNING);
    std::cout << "[PASS] test_countdown_manual_start\n";
}

void test_countdown_led_1hz_blink() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    countdownInit();

    mockState.currentMillis = 1000;
    countdownStart();
    countdownUpdate();

    // 0 ms into countdown: LED should be HIGH
    assert(mockState.digitalPins[PIN_LED] == HIGH);

    // 499 ms: still HIGH
    mockState.currentMillis = 1499;
    countdownUpdate();
    assert(mockState.digitalPins[PIN_LED] == HIGH);

    // 500 ms: toggles to LOW (1 Hz has 500ms half-period)
    mockState.currentMillis = 1500;
    countdownUpdate();
    assert(mockState.digitalPins[PIN_LED] == LOW);

    // 1000 ms: toggles back to HIGH
    mockState.currentMillis = 2000;
    countdownUpdate();
    assert(mockState.digitalPins[PIN_LED] == HIGH);

    // 1500 ms: toggles to LOW
    mockState.currentMillis = 2500;
    countdownUpdate();
    assert(mockState.digitalPins[PIN_LED] == LOW);

    std::cout << "[PASS] test_countdown_led_1hz_blink\n";
}

void test_countdown_completion_and_solid_led() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    countdownInit();

    mockState.currentMillis = 1000;
    countdownStart();

    // Advance 4999 ms: still running
    mockState.currentMillis = 5999;
    countdownUpdate();
    assert(countdownIsRunning() == true);
    assert(countdownIsComplete() == false);

    // Exactly 5000 ms: complete
    mockState.currentMillis = 6000;
    countdownUpdate();
    assert(countdownIsRunning() == false);
    assert(countdownIsComplete() == true);
    assert(countdownGetState() == COUNTDOWN_COMPLETE);
    // In combat, LED must remain solidly HIGH
    assert(mockState.digitalPins[PIN_LED] == HIGH);

    // Further updates during combat keep LED solidly HIGH
    mockState.currentMillis = 7500;
    countdownUpdate();
    assert(mockState.digitalPins[PIN_LED] == HIGH);

    std::cout << "[PASS] test_countdown_completion_and_solid_led\n";
}

void test_countdown_ignores_button_while_running() {
    mockState.reset();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    countdownInit();

    mockState.currentMillis = 1000;
    countdownStart();

    // Advance to 3000 ms (elapsed 2000 ms, 3000 ms remaining)
    mockState.currentMillis = 3000;
    countdownUpdate();
    assert(countdownGetRemainingMs() == 3000);

    // Press button again
    mockState.digitalPins[PIN_BUTTON] = LOW;
    countdownUpdate();

    // Remaining time must NOT reset to 5000
    assert(countdownGetRemainingMs() == 3000);
    std::cout << "[PASS] test_countdown_ignores_button_while_running\n";
}

int main() {
    std::cout << "--- Running Countdown Sequencer Unit Tests ---\n";
    test_countdown_init();
    test_countdown_button_trigger();
    test_countdown_manual_start();
    test_countdown_led_1hz_blink();
    test_countdown_completion_and_solid_led();
    test_countdown_ignores_button_while_running();
    std::cout << "All countdown tests passed successfully!\n";
    return 0;
}
