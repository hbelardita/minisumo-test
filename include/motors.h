#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

/**
 * Configure TB6612FNG motor control pins as OUTPUT and initialize in coast mode.
 */
void motorsInit(void);

/**
 * Set motor speeds for left and right channels (-255 to 255).
 * Positive speeds drive forward, negative speeds drive reverse.
 * Maps polarity according to physical wiring.
 * A speed of 0 puts the channel into high-impedance coasting.
 */
void motorsSetSpeed(int leftSpeed, int rightSpeed);

/**
 * Places the TB6612 driver in high-impedance mode (all motor outputs floating).
 * Used during zero-speed pauses and Border Avoidance initial coasting (ADR-0001).
 */
void motorsCoast(void);

/**
 * Actively brakes both motors by shorting motor terminals to ground.
 */
void motorsBrake(void);

#endif // MOTORS_H
