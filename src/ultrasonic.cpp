#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#endif
#include "pins.h"
#include "ultrasonic.h"

static volatile unsigned long s_echoStartUs = 0;
static volatile unsigned long s_pulseDurationUs = 0;
static volatile bool s_echoActive = false;
static volatile bool s_newEchoReady = false;

static unsigned long s_lastPingMs = 0;
static bool s_hasPinged = false;
static bool s_waitingForEcho = false;
static int s_currentDistance = NO_OPPONENT;
static bool s_hasOpponent = false;

void ultrasonicInit(void) {
    pinMode(PIN_TRIG, OUTPUT);
    digitalWrite(PIN_TRIG, LOW);
    pinMode(PIN_ECHO, INPUT);

    s_echoStartUs = 0;
    s_pulseDurationUs = 0;
    s_echoActive = false;
    s_newEchoReady = false;

    s_lastPingMs = 0;
    s_hasPinged = false;
    s_waitingForEcho = false;
    s_currentDistance = NO_OPPONENT;
    s_hasOpponent = false;

    // Enable Pin Change Interrupt 0 for PCINT4 (Pin 12)
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT4);
}

void ultrasonicTriggerPing(void) {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    s_lastPingMs = millis();
    s_hasPinged = true;
    s_waitingForEcho = true;
    s_echoActive = false;
}

void ultrasonicEchoIsr(void) {
    int echoVal = digitalRead(PIN_ECHO);
    unsigned long nowUs = micros();

    if (echoVal == HIGH) {
        s_echoStartUs = nowUs;
        s_echoActive = true;
    } else if (s_echoActive) {
        s_pulseDurationUs = nowUs - s_echoStartUs;
        s_echoActive = false;
        s_newEchoReady = true;
    }
}

#if !defined(UNIT_TEST) && defined(ARDUINO) && defined(__AVR__)
ISR(PCINT0_vect) {
    ultrasonicEchoIsr();
}
#endif

void ultrasonicUpdate(void) {
    unsigned long now = millis();

    if (s_newEchoReady) {
        noInterrupts();
        s_newEchoReady = false;
        s_waitingForEcho = false;
        unsigned long duration = s_pulseDurationUs;
        interrupts();

        int dist = (int)(duration / 58UL);

        if (dist > 0 && dist <= ULTRASONIC_MAX_DISTANCE_CM) {
            s_currentDistance = dist;
            s_hasOpponent = true;
        } else {
            s_currentDistance = NO_OPPONENT;
            s_hasOpponent = false;
        }
    } else if (s_waitingForEcho && (now - s_lastPingMs >= ULTRASONIC_PING_INTERVAL_MS)) {
        s_waitingForEcho = false;
        s_currentDistance = NO_OPPONENT;
        s_hasOpponent = false;
    }

    if (!s_hasPinged || (now - s_lastPingMs >= ULTRASONIC_PING_INTERVAL_MS)) {
        ultrasonicTriggerPing();
    }
}

int ultrasonicGetDistance(void) {
    return s_currentDistance;
}

bool ultrasonicHasOpponent(void) {
    return s_hasOpponent;
}
