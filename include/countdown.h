#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <stdint.h>
#include <stdbool.h>

#define COUNTDOWN_DURATION_MS 5000UL
#define COUNTDOWN_LED_BLINK_INTERVAL_MS 500UL

enum CountdownState {
    COUNTDOWN_IDLE,
    COUNTDOWN_RUNNING,
    COUNTDOWN_COMPLETE
};

/**
 * Configure button pin as INPUT_PULLUP and LED pin as OUTPUT.
 */
void countdownInit(void);

/**
 * Manually trigger the start of the 5000 ms countdown.
 */
void countdownStart(void);

/**
 * Non-blocking loop update: checks button trigger, tracks remaining time,
 * toggles LED at 1 Hz during countdown, and drives LED solid ON upon completion.
 */
void countdownUpdate(void);

/**
 * Returns true if countdown is actively running.
 */
bool countdownIsRunning(void);

/**
 * Returns true if countdown has completed and robot is in active combat.
 */
bool countdownIsComplete(void);

/**
 * Returns current countdown state enum.
 */
CountdownState countdownGetState(void);

/**
 * Returns remaining countdown time in milliseconds.
 */
unsigned long countdownGetRemainingMs(void);

#endif // COUNTDOWN_H
