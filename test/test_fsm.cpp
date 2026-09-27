#include "mock_arduino.h"
#include "pins.h"
#include "motors.h"
#include "countdown.h"
#include "line_sensors.h"
#include "ultrasonic.h"
#include "fsm.h"
#include <cassert>
#include <iostream>

ArduinoMockState mockState;

void resetHarness() {
    mockState.reset();
    motorsInit();
    lineSensorsInit();
    ultrasonicInit();
    countdownInit();
    // Default: sensors on Dohyo (black surface), no opponent
    mockState.analogReadValues[PIN_TCRT_LEFT] = 800;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 800;
    fsmInit();
}

static void simulateUltrasonicDistance(int cm) {
    mockState.digitalPins[PIN_ECHO] = HIGH;
    ultrasonicEchoIsr();
    mockState.currentMicros += (cm * 58);
    mockState.digitalPins[PIN_ECHO] = LOW;
    ultrasonicEchoIsr();
    ultrasonicUpdate();
}

void test_fsm_init() {
    resetHarness();
    assert(fsmGetState() == STATE_IDLE);
    // Motors should be stopped / coasting
    assert(mockState.digitalPins[PIN_STBY] == LOW);
    std::cout << "[PASS] test_fsm_init\n";
}

void test_fsm_idle_to_countdown_and_search() {
    resetHarness();
    mockState.digitalPins[PIN_BUTTON] = HIGH;
    fsmInit();
    fsmUpdate();
    assert(fsmGetState() == STATE_IDLE);

    // Start countdown
    countdownStart();
    fsmUpdate();
    assert(fsmGetState() == STATE_COUNTDOWN);
    // In countdown, motors must remain coasting / 0
    assert(mockState.digitalPins[PIN_STBY] == LOW);

    // Advance time past 5 seconds (5000 ms)
    mockState.currentMillis = 5000;
    countdownUpdate();
    assert(countdownIsComplete() == true);

    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    std::cout << "[PASS] test_fsm_idle_to_countdown_and_search\n";
}

void test_fsm_search_pivot_scan() {
    resetHarness();
    fsmSetState(STATE_SEARCH);

    fsmUpdate();

    assert(fsmGetState() == STATE_SEARCH);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Pivot Scan: left 130, right -130
    // Right motor (PWMA): 130, reverse direction AIN1=HIGH, AIN2=LOW
    assert(mockState.analogPins[PIN_PWMA] == 130);
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);

    // Left motor (PWMB): 130, forward direction BIN1=LOW, BIN2=HIGH
    assert(mockState.analogPins[PIN_PWMB] == 130);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);
    std::cout << "[PASS] test_fsm_search_pivot_scan\n";
}

void test_fsm_search_to_approach_and_lost() {
    resetHarness();
    fsmSetState(STATE_SEARCH);

    // Opponent detected at 45 cm (in range [30, 65])
    mockState.currentMicros = 100;
    simulateUltrasonicDistance(45);
    assert(ultrasonicGetDistance() == 45);

    fsmUpdate();
    assert(fsmGetState() == STATE_APPROACH);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Forward at 150 PWM: both motors positive 150
    // Right motor (PWMA): 150, forward AIN1=LOW, AIN2=HIGH
    assert(mockState.analogPins[PIN_PWMA] == 150);
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == HIGH);
    // Left motor (PWMB): 150, forward BIN1=LOW, BIN2=HIGH
    assert(mockState.analogPins[PIN_PWMB] == 150);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);

    // Opponent lost: trigger ping with no echo and advance time
    mockState.currentMillis += 50;
    mockState.currentMicros += 50000;
    ultrasonicTriggerPing();
    mockState.currentMillis += 45;
    mockState.currentMicros += 45000;
    ultrasonicUpdate();
    assert(ultrasonicGetDistance() == NO_OPPONENT);

    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    assert(mockState.analogPins[PIN_PWMA] == 130);
    assert(mockState.analogPins[PIN_PWMB] == 130);
    std::cout << "[PASS] test_fsm_search_to_approach_and_lost\n";
}

void test_fsm_approach_to_attack_and_transitions() {
    resetHarness();
    fsmSetState(STATE_APPROACH);

    // Opponent closes distance to 20 cm (< 30 cm)
    mockState.currentMicros = 100;
    simulateUltrasonicDistance(20);
    assert(ultrasonicGetDistance() == 20);

    fsmUpdate();
    assert(fsmGetState() == STATE_ATTACK);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Full forward 100% PWM (255)
    // Right motor (PWMA): 255, forward AIN1=LOW, AIN2=HIGH
    assert(mockState.analogPins[PIN_PWMA] == 255);
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == HIGH);
    // Left motor (PWMB): 255, forward BIN1=LOW, BIN2=HIGH
    assert(mockState.analogPins[PIN_PWMB] == 255);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);

    // Opponent distance remains within engagement (< 65 cm), say 40 cm -> robot stays in ATTACK
    mockState.currentMicros += 50000;
    simulateUltrasonicDistance(40);
    assert(ultrasonicGetDistance() == 40);

    fsmUpdate();
    assert(fsmGetState() == STATE_ATTACK);
    assert(mockState.analogPins[PIN_PWMA] == 255);
    assert(mockState.analogPins[PIN_PWMB] == 255);

    // Opponent jumps directly to 10 cm from SEARCH -> transitions directly to ATTACK
    fsmSetState(STATE_SEARCH);
    mockState.currentMicros += 50000;
    simulateUltrasonicDistance(10);
    assert(ultrasonicGetDistance() == 10);

    fsmUpdate();
    assert(fsmGetState() == STATE_ATTACK);
    assert(mockState.analogPins[PIN_PWMA] == 255);
    assert(mockState.analogPins[PIN_PWMB] == 255);

    // Opponent at boundary of 30 cm triggers ATTACK from APPROACH
    fsmSetState(STATE_APPROACH);
    mockState.currentMicros += 50000;
    simulateUltrasonicDistance(30);
    assert(ultrasonicGetDistance() == 30);
    fsmUpdate();
    assert(fsmGetState() == STATE_ATTACK);

    // Opponent lost completely while in ATTACK -> reverts to SEARCH
    mockState.currentMillis += 50;
    mockState.currentMicros += 50000;
    ultrasonicTriggerPing();
    mockState.currentMillis += 45;
    mockState.currentMicros += 45000;
    ultrasonicUpdate();
    assert(ultrasonicGetDistance() == NO_OPPONENT);

    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    assert(mockState.analogPins[PIN_PWMA] == 130);
    assert(mockState.analogPins[PIN_PWMB] == 130);
    std::cout << "[PASS] test_fsm_approach_to_attack_and_transitions\n";
}

void test_fsm_border_avoid_preemption_and_sequence_left() {
    resetHarness();
    fsmSetState(STATE_ATTACK);

    mockState.currentMillis = 1000;
    // Left sensor hits border (< 400)
    mockState.analogReadValues[PIN_TCRT_LEFT] = 200;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 800;

    fsmUpdate();
    // Preempts attack immediately
    assert(fsmGetState() == STATE_BORDER_AVOIDANCE);
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_COAST);
    // Coast phase: TB6612 standby LOW, all outputs 0
    assert(mockState.digitalPins[PIN_STBY] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 0);
    assert(mockState.analogPins[PIN_PWMB] == 0);

    // 39 ms later: still coasting
    mockState.currentMillis = 1039;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_COAST);
    assert(mockState.digitalPins[PIN_STBY] == LOW);

    // 40 ms after trigger (1040 ms): transitions to BORDER_AVOIDANCE_REVERSE
    mockState.currentMillis = 1040;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_REVERSE);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Reverse full power (-255, -255)
    // Right motor reverse: AIN1=HIGH, AIN2=LOW, PWMA=255
    assert(mockState.analogPins[PIN_PWMA] == 255);
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);
    // Left motor reverse: BIN1=HIGH, BIN2=LOW, PWMB=255
    assert(mockState.analogPins[PIN_PWMB] == 255);
    assert(mockState.digitalPins[PIN_BIN1] == HIGH);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);

    // 339 ms after trigger (299 ms into reverse): still reversing
    mockState.currentMillis = 1339;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_REVERSE);

    // 340 ms after trigger (40 + 300 ms): transitions to BORDER_AVOIDANCE_PIVOT
    // Since left sensor triggered, turn right: left forward (+200), right reverse (-200)
    mockState.currentMillis = 1340;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_PIVOT);
    assert(mockState.digitalPins[PIN_STBY] == HIGH);
    // Left motor forward: 200, BIN1=LOW, BIN2=HIGH
    assert(mockState.analogPins[PIN_PWMB] == 200);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);
    // Right motor reverse: 200, AIN1=HIGH, AIN2=LOW
    assert(mockState.analogPins[PIN_PWMA] == 200);
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);

    // 559 ms after trigger: still pivoting
    mockState.currentMillis = 1559;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_PIVOT);

    // 560 ms after trigger (40 + 300 + 220 ms): completes avoidance and returns to STATE_SEARCH
    // Sensor is back on Dohyo
    mockState.analogReadValues[PIN_TCRT_LEFT] = 800;
    mockState.currentMillis = 1560;
    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    assert(mockState.analogPins[PIN_PWMA] == 130);
    assert(mockState.analogPins[PIN_PWMB] == 130);
    std::cout << "[PASS] test_fsm_border_avoid_preemption_and_sequence_left\n";
}

void test_fsm_border_avoid_right_sensor_turns_left() {
    resetHarness();
    fsmSetState(STATE_SEARCH);

    mockState.currentMillis = 2000;
    // Right sensor hits border (< 400)
    mockState.analogReadValues[PIN_TCRT_LEFT] = 800;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 150;

    fsmUpdate();
    assert(fsmGetState() == STATE_BORDER_AVOIDANCE);
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_COAST);

    // Advance past Coast (40 ms)
    mockState.currentMillis = 2040;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_REVERSE);

    // Advance past Reverse (300 ms) -> Pivot phase
    mockState.currentMillis = 2340;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_PIVOT);
    // Right sensor triggered -> turn left: left reverse (-200), right forward (+200)
    assert(mockState.analogPins[PIN_PWMB] == 200);
    assert(mockState.digitalPins[PIN_BIN1] == HIGH);
    assert(mockState.digitalPins[PIN_BIN2] == LOW);
    assert(mockState.analogPins[PIN_PWMA] == 200);
    assert(mockState.digitalPins[PIN_AIN1] == LOW);
    assert(mockState.digitalPins[PIN_AIN2] == HIGH);

    // Advance past Pivot (220 ms)
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 800;
    mockState.currentMillis = 2560;
    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    std::cout << "[PASS] test_fsm_border_avoid_right_sensor_turns_left\n";
}

void test_fsm_border_avoid_both_sensors_spins() {
    resetHarness();
    fsmSetState(STATE_APPROACH);

    mockState.currentMillis = 3000;
    // Both sensors hit border
    mockState.analogReadValues[PIN_TCRT_LEFT] = 250;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 250;

    fsmUpdate();
    assert(fsmGetState() == STATE_BORDER_AVOIDANCE);
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_COAST);

    // Advance past Coast (40 ms)
    mockState.currentMillis = 3040;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_REVERSE);

    // Advance past Reverse (300 ms)
    mockState.currentMillis = 3340;
    fsmUpdate();
    assert(fsmGetBorderAvoidancePhase() == BORDER_AVOIDANCE_PIVOT);
    // Both triggered -> Spin 180 degrees (200, -200)
    assert(mockState.analogPins[PIN_PWMB] == 200);
    assert(mockState.digitalPins[PIN_BIN1] == LOW);
    assert(mockState.digitalPins[PIN_BIN2] == HIGH);
    assert(mockState.analogPins[PIN_PWMA] == 200);
    assert(mockState.digitalPins[PIN_AIN1] == HIGH);
    assert(mockState.digitalPins[PIN_AIN2] == LOW);

    // Advance past Pivot
    mockState.analogReadValues[PIN_TCRT_LEFT] = 800;
    mockState.analogReadValues[PIN_TCRT_RIGHT] = 800;
    mockState.currentMillis = 3560;
    fsmUpdate();
    assert(fsmGetState() == STATE_SEARCH);
    std::cout << "[PASS] test_fsm_border_avoid_both_sensors_spins\n";
}

int main() {
    std::cout << "--- Running Combat FSM Unit Tests ---\n";
    test_fsm_init();
    test_fsm_idle_to_countdown_and_search();
    test_fsm_search_pivot_scan();
    test_fsm_search_to_approach_and_lost();
    test_fsm_approach_to_attack_and_transitions();
    test_fsm_border_avoid_preemption_and_sequence_left();
    test_fsm_border_avoid_right_sensor_turns_left();
    test_fsm_border_avoid_both_sensors_spins();
    std::cout << "All FSM tests passed successfully!\n";
    return 0;
}
