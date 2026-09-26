#include "mock_arduino.h"
#include "pins.h"
#include "ultrasonic.h"
#include <cassert>
#include <iostream>

ArduinoMockState mockState;

void simulateEchoPulse(unsigned long pulseUs) {
    // Rising edge
    mockState.digitalPins[PIN_ECHO] = HIGH;
    ultrasonicEchoIsr();

    // Advance time by pulse width
    mockState.currentMicros += pulseUs;

    // Falling edge
    mockState.digitalPins[PIN_ECHO] = LOW;
    ultrasonicEchoIsr();
}

void test_ultrasonic_init() {
    mockState.reset();
    ultrasonicInit();

    assert(mockState.pinModes[PIN_TRIG] == OUTPUT);
    assert(mockState.pinModes[PIN_ECHO] == INPUT);
    assert(mockState.digitalPins[PIN_TRIG] == LOW);
    assert(ultrasonicGetDistance() == NO_OPPONENT);
    assert(ultrasonicHasOpponent() == false);
    std::cout << "[PASS] test_ultrasonic_init\n";
}

void test_ultrasonic_trigger_pulse() {
    mockState.reset();
    ultrasonicInit();

    ultrasonicTriggerPing();
    // After ping, TRIG should return to LOW and micros should have advanced ~12us (2us low + 10us high)
    assert(mockState.digitalPins[PIN_TRIG] == LOW);
    assert(mockState.currentMicros >= 10);
    std::cout << "[PASS] test_ultrasonic_trigger_pulse\n";
}

void test_ultrasonic_valid_echo_opponent() {
    mockState.reset();
    ultrasonicInit();

    // Ping at t=0
    ultrasonicTriggerPing();

    // Opponent at 20 cm -> echo pulse is 20 * 58 = 1160 us
    mockState.currentMicros = 500; // 500 us after trigger
    simulateEchoPulse(1160);

    ultrasonicUpdate();

    assert(ultrasonicHasOpponent() == true);
    assert(ultrasonicGetDistance() == 20);
    std::cout << "[PASS] test_ultrasonic_valid_echo_opponent\n";
}

void test_ultrasonic_max_range_boundary() {
    mockState.reset();
    ultrasonicInit();

    // Exactly 65 cm -> 65 * 58 = 3770 us
    ultrasonicTriggerPing();
    mockState.currentMicros = 200;
    simulateEchoPulse(3770);
    ultrasonicUpdate();

    assert(ultrasonicHasOpponent() == true);
    assert(ultrasonicGetDistance() == 65);
    std::cout << "[PASS] test_ultrasonic_max_range_boundary\n";
}

void test_ultrasonic_filter_beyond_65cm() {
    mockState.reset();
    ultrasonicInit();

    // Opponent beyond 65 cm (e.g. 66 cm -> 66 * 58 = 3828 us)
    ultrasonicTriggerPing();
    mockState.currentMicros = 200;
    simulateEchoPulse(3828);
    ultrasonicUpdate();

    assert(ultrasonicHasOpponent() == false);
    assert(ultrasonicGetDistance() == NO_OPPONENT);

    // Opponent at 100 cm (5800 us)
    ultrasonicTriggerPing();
    mockState.currentMicros = 10000;
    simulateEchoPulse(5800);
    ultrasonicUpdate();

    assert(ultrasonicHasOpponent() == false);
    assert(ultrasonicGetDistance() == NO_OPPONENT);
    std::cout << "[PASS] test_ultrasonic_filter_beyond_65cm\n";
}

void test_ultrasonic_timeout_filters_no_opponent() {
    mockState.reset();
    ultrasonicInit();

    // First detect a valid Opponent at 15 cm
    ultrasonicTriggerPing();
    mockState.currentMicros = 100;
    simulateEchoPulse(15 * 58);
    ultrasonicUpdate();
    assert(ultrasonicHasOpponent() == true);
    assert(ultrasonicGetDistance() == 15);

    // Now trigger ping, but no echo returns (timed out ping)
    mockState.currentMillis = 50;
    mockState.currentMicros = 50000;
    ultrasonicTriggerPing();

    // Advance time past 40ms without receiving echo
    mockState.currentMillis += 45;
    mockState.currentMicros += 45000;
    ultrasonicUpdate();

    assert(ultrasonicHasOpponent() == false);
    assert(ultrasonicGetDistance() == NO_OPPONENT);
    std::cout << "[PASS] test_ultrasonic_timeout_filters_no_opponent\n";
}

void test_ultrasonic_periodic_scheduling() {
    mockState.reset();
    ultrasonicInit();

    mockState.currentMillis = 0;
    ultrasonicUpdate(); // should trigger first ping at t=0

    // Advance 20ms: should NOT trigger another ping yet
    unsigned long microsBefore = mockState.currentMicros;
    mockState.currentMillis = 20;
    ultrasonicUpdate();
    assert(mockState.currentMicros == microsBefore);

    // Advance to 40ms: should trigger next ping
    mockState.currentMillis = 40;
    ultrasonicUpdate();
    assert(mockState.currentMicros > microsBefore);
    std::cout << "[PASS] test_ultrasonic_periodic_scheduling\n";
}

int main() {
    std::cout << "--- Running Ultrasonic Sensor Unit Tests ---\n";
    test_ultrasonic_init();
    test_ultrasonic_trigger_pulse();
    test_ultrasonic_valid_echo_opponent();
    test_ultrasonic_max_range_boundary();
    test_ultrasonic_filter_beyond_65cm();
    test_ultrasonic_timeout_filters_no_opponent();
    test_ultrasonic_periodic_scheduling();
    std::cout << "All ultrasonic tests passed successfully!\n";
    return 0;
}
