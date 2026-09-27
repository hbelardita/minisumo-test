#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include <stdbool.h>

#define PIVOT_SCAN_PWM 130
#define APPROACH_PWM 150
#define ATTACK_PWM 255
#define AVOID_REVERSE_PWM 255
#define AVOID_PIVOT_PWM 200

#define BORDER_COAST_DURATION_MS 40UL
#define BORDER_REVERSE_DURATION_MS 300UL
#define BORDER_PIVOT_DURATION_MS 220UL

#define OPPONENT_ATTACK_MAX_CM 30
#define OPPONENT_APPROACH_MAX_CM 65

enum MatchState {
    STATE_IDLE,
    STATE_COUNTDOWN,
    STATE_SEARCH,
    STATE_APPROACH,
    STATE_ATTACK,
    STATE_BORDER_AVOIDANCE,
    STATE_BORDER_AVOID = STATE_BORDER_AVOIDANCE
};

enum BorderAvoidancePhase {
    BORDER_AVOIDANCE_COAST,
    BORDER_AVOIDANCE_REVERSE,
    BORDER_AVOIDANCE_PIVOT
};

/**
 * Initialize match finite state machine to STATE_IDLE and motors to coast.
 */
void fsmInit(void);

/**
 * Cooperative FSM update step.
 * Reads countdown, line sensors, and ultrasonic rangefinder,
 * and updates state and motor speed outputs.
 */
void fsmUpdate(void);

/**
 * Returns current match state.
 */
MatchState fsmGetState(void);

/**
 * Returns current phase within STATE_BORDER_AVOIDANCE (if in STATE_BORDER_AVOIDANCE).
 */
BorderAvoidancePhase fsmGetBorderAvoidancePhase(void);

/**
 * Explicitly set match state (primarily for test harness setup or resets).
 */
void fsmSetState(MatchState state);

#endif // FSM_H
