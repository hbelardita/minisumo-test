#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>
#include <stdbool.h>

#define ULTRASONIC_MAX_DISTANCE_CM 65
#define ULTRASONIC_PING_INTERVAL_MS 40
#define NO_OPPONENT -1

/**
 * Configure HC-SR04 pins and enable Pin Change Interrupts on PCINT4 (Pin 12).
 */
void ultrasonicInit(void);

/**
 * Send 10 us trigger pulse on PIN_TRIG to initiate ultrasonic ping.
 */
void ultrasonicTriggerPing(void);

/**
 * Interrupt Service Routine handler for PCINT0_vect on Pin 12 (PIN_ECHO).
 */
void ultrasonicEchoIsr(void);

/**
 * Non-blocking periodic scheduler and echo processor.
 * Should be called frequently in loop().
 */
void ultrasonicUpdate(void);

/**
 * Returns latest measured Opponent distance in centimeters (1..65),
 * or NO_OPPONENT (-1) if out of bounds or timed out.
 */
int ultrasonicGetDistance(void);

/**
 * Returns true if an Opponent is detected within 65 cm.
 */
bool ultrasonicHasOpponent(void);

#endif // ULTRASONIC_H
