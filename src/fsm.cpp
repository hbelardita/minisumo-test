#include "fsm.h"
#include "motors.h"
#include "countdown.h"
#include "line_sensors.h"
#include "ultrasonic.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "mock_arduino.h"
#endif

static MatchState currentState = STATE_IDLE;
static BorderAvoidancePhase avoidPhase = BORDER_AVOIDANCE_COAST;
static unsigned long avoidPhaseStartTime = 0;
static LineSensorsReading latchedBorders = {false, false, 0, 0};

static void applyAvoidancePivot(void) {
    if (latchedBorders.leftBorder && latchedBorders.rightBorder) {
        // Both sensors triggered -> Spin 180 degrees (left forward, right reverse)
        motorsSetSpeed(AVOID_PIVOT_PWM, -AVOID_PIVOT_PWM);
    } else if (latchedBorders.leftBorder) {
        // Left sensor triggered -> Turn right away from border (left forward, right reverse)
        motorsSetSpeed(AVOID_PIVOT_PWM, -AVOID_PIVOT_PWM);
    } else {
        // Right sensor triggered -> Turn left away from border (left reverse, right forward)
        motorsSetSpeed(-AVOID_PIVOT_PWM, AVOID_PIVOT_PWM);
    }
}

static bool isOpponentLost(int dist) {
    return dist == NO_OPPONENT || dist > OPPONENT_APPROACH_MAX_CM;
}

static void transitionToSearch(void) {
    currentState = STATE_SEARCH;
    motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
}

void fsmInit(void) {
    currentState = STATE_IDLE;
    avoidPhase = BORDER_AVOIDANCE_COAST;
    avoidPhaseStartTime = 0;
    latchedBorders.leftBorder = false;
    latchedBorders.rightBorder = false;
    motorsCoast();
}

MatchState fsmGetState(void) {
    return currentState;
}

BorderAvoidancePhase fsmGetBorderAvoidancePhase(void) {
    return avoidPhase;
}

void fsmSetState(MatchState state) {
    currentState = state;
}

static void triggerBorderAvoidance(LineSensorsReading lines) {
    currentState = STATE_BORDER_AVOIDANCE;
    avoidPhase = BORDER_AVOIDANCE_COAST;
    avoidPhaseStartTime = millis();
    latchedBorders = lines;
    motorsCoast();
}

void fsmUpdate(void) {
    LineSensorsReading lines = lineSensorsRead();

    // High-priority border avoidance preemption during active match states
    if (currentState == STATE_SEARCH || currentState == STATE_APPROACH || currentState == STATE_ATTACK) {
        if (lines.leftBorder || lines.rightBorder) {
            triggerBorderAvoidance(lines);
            return;
        }
    }

    switch (currentState) {
        case STATE_IDLE:
            motorsCoast();
            if (countdownIsRunning()) {
                currentState = STATE_COUNTDOWN;
            }
            break;

        case STATE_COUNTDOWN:
            motorsCoast();
            if (countdownIsComplete()) {
                currentState = STATE_SEARCH;
            }
            break;

        case STATE_SEARCH: {
            int dist = ultrasonicGetDistance();
            if (dist != NO_OPPONENT && dist <= OPPONENT_APPROACH_MAX_CM) {
                if (dist < OPPONENT_ATTACK_MAX_CM) {
                    currentState = STATE_ATTACK;
                    motorsSetSpeed(ATTACK_PWM, ATTACK_PWM);
                } else {
                    currentState = STATE_APPROACH;
                    motorsSetSpeed(APPROACH_PWM, APPROACH_PWM);
                }
            } else {
                motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
            }
            break;
        }

        case STATE_APPROACH: {
            int dist = ultrasonicGetDistance();
            if (isOpponentLost(dist)) {
                transitionToSearch();
            } else if (dist < OPPONENT_ATTACK_MAX_CM) {
                currentState = STATE_ATTACK;
                motorsSetSpeed(ATTACK_PWM, ATTACK_PWM);
            } else {
                motorsSetSpeed(APPROACH_PWM, APPROACH_PWM);
            }
            break;
        }

        case STATE_ATTACK: {
            int dist = ultrasonicGetDistance();
            if (isOpponentLost(dist)) {
                transitionToSearch();
            } else {
                motorsSetSpeed(ATTACK_PWM, ATTACK_PWM);
            }
            break;
        }

        case STATE_BORDER_AVOIDANCE: {
            unsigned long elapsed = millis() - avoidPhaseStartTime;
            switch (avoidPhase) {
                case BORDER_AVOIDANCE_COAST:
                    motorsCoast();
                    if (elapsed >= BORDER_COAST_DURATION_MS) {
                        avoidPhase = BORDER_AVOIDANCE_REVERSE;
                        avoidPhaseStartTime = millis();
                        motorsSetSpeed(-AVOID_REVERSE_PWM, -AVOID_REVERSE_PWM);
                    }
                    break;

                case BORDER_AVOIDANCE_REVERSE:
                    motorsSetSpeed(-AVOID_REVERSE_PWM, -AVOID_REVERSE_PWM);
                    if (elapsed >= BORDER_REVERSE_DURATION_MS) {
                        avoidPhase = BORDER_AVOIDANCE_PIVOT;
                        avoidPhaseStartTime = millis();
                        applyAvoidancePivot();
                    }
                    break;

                case BORDER_AVOIDANCE_PIVOT:
                    applyAvoidancePivot();
                    if (elapsed >= BORDER_PIVOT_DURATION_MS) {
                        transitionToSearch();
                    }
                    break;
            }
            break;
        }
    }
}
