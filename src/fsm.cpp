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
static BorderAvoidPhase avoidPhase = AVOID_COAST;
static unsigned long avoidPhaseStartTime = 0;
static bool latchedLeftBorder = false;
static bool latchedRightBorder = false;

static void applyAvoidancePivot(void) {
    if (latchedLeftBorder && latchedRightBorder) {
        // Spin 180 degrees (both sensors hit)
        motorsSetSpeed(AVOID_PIVOT_PWM, -AVOID_PIVOT_PWM);
    } else if (latchedLeftBorder) {
        // Left sensor -> Turn right
        motorsSetSpeed(AVOID_PIVOT_PWM, -AVOID_PIVOT_PWM);
    } else {
        // Right sensor -> Turn left
        motorsSetSpeed(-AVOID_PIVOT_PWM, AVOID_PIVOT_PWM);
    }
}

void fsmInit(void) {
    currentState = STATE_IDLE;
    avoidPhase = AVOID_COAST;
    avoidPhaseStartTime = 0;
    latchedLeftBorder = false;
    latchedRightBorder = false;
    motorsCoast();
}

MatchState fsmGetState(void) {
    return currentState;
}

BorderAvoidPhase fsmGetBorderAvoidPhase(void) {
    return avoidPhase;
}

void fsmSetState(MatchState state) {
    currentState = state;
}

static void triggerBorderAvoid(bool left, bool right) {
    currentState = STATE_BORDER_AVOID;
    avoidPhase = AVOID_COAST;
    avoidPhaseStartTime = millis();
    latchedLeftBorder = left;
    latchedRightBorder = right;
    motorsCoast();
}

void fsmUpdate(void) {
    LineSensorsReading lines = lineSensorsRead();

    // High-priority border avoidance preemption during active match states
    if (currentState == STATE_SEARCH || currentState == STATE_APPROACH || currentState == STATE_ATTACK) {
        if (lines.leftBorder || lines.rightBorder) {
            triggerBorderAvoid(lines.leftBorder, lines.rightBorder);
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
            if (dist >= 0 && dist <= OPPONENT_APPROACH_MAX_CM) {
                currentState = STATE_APPROACH;
                motorsSetSpeed(APPROACH_PWM, APPROACH_PWM);
            } else {
                motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
            }
            break;
        }

        case STATE_APPROACH: {
            int dist = ultrasonicGetDistance();
            if (dist < 0 || dist > OPPONENT_APPROACH_MAX_CM) {
                currentState = STATE_SEARCH;
                motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
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
            if (dist < 0 || dist > OPPONENT_APPROACH_MAX_CM) {
                currentState = STATE_SEARCH;
                motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
            } else if (dist >= OPPONENT_ATTACK_MAX_CM) {
                currentState = STATE_APPROACH;
                motorsSetSpeed(APPROACH_PWM, APPROACH_PWM);
            } else {
                motorsSetSpeed(ATTACK_PWM, ATTACK_PWM);
            }
            break;
        }

        case STATE_BORDER_AVOID: {
            unsigned long elapsed = millis() - avoidPhaseStartTime;
            switch (avoidPhase) {
                case AVOID_COAST:
                    motorsCoast();
                    if (elapsed >= BORDER_COAST_DURATION_MS) {
                        avoidPhase = AVOID_REVERSE;
                        avoidPhaseStartTime = millis();
                        motorsSetSpeed(-AVOID_REVERSE_PWM, -AVOID_REVERSE_PWM);
                    }
                    break;

                case AVOID_REVERSE:
                    motorsSetSpeed(-AVOID_REVERSE_PWM, -AVOID_REVERSE_PWM);
                    if (elapsed >= BORDER_REVERSE_DURATION_MS) {
                        avoidPhase = AVOID_PIVOT;
                        avoidPhaseStartTime = millis();
                        applyAvoidancePivot();
                    }
                    break;

                case AVOID_PIVOT:
                    applyAvoidancePivot();
                    if (elapsed >= BORDER_PIVOT_DURATION_MS) {
                        currentState = STATE_SEARCH;
                        motorsSetSpeed(PIVOT_SCAN_PWM, -PIVOT_SCAN_PWM);
                    }
                    break;
            }
            break;
        }
    }
}
