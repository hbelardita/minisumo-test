#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "countdown.h"

static CountdownState s_state = COUNTDOWN_IDLE;
static unsigned long s_startTimestamp = 0;

void countdownInit(void) {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    s_state = COUNTDOWN_IDLE;
    s_startTimestamp = 0;
}

void countdownStart(void) {
    if (s_state == COUNTDOWN_IDLE) {
        s_startTimestamp = millis();
        s_state = COUNTDOWN_RUNNING;
        digitalWrite(PIN_LED, HIGH);
    }
}

void countdownUpdate(void) {
    if (s_state == COUNTDOWN_IDLE) {
        if (digitalRead(PIN_BUTTON) == LOW) {
            countdownStart();
        }
    } else if (s_state == COUNTDOWN_RUNNING) {
        unsigned long elapsed = millis() - s_startTimestamp;
        if (elapsed >= COUNTDOWN_DURATION_MS) {
            s_state = COUNTDOWN_COMPLETE;
            digitalWrite(PIN_LED, HIGH);
        } else {
            uint8_t ledVal = ((elapsed / COUNTDOWN_LED_BLINK_INTERVAL_MS) % 2 == 0) ? HIGH : LOW;
            digitalWrite(PIN_LED, ledVal);
        }
    } else if (s_state == COUNTDOWN_COMPLETE) {
        digitalWrite(PIN_LED, HIGH);
    }
}

bool countdownIsRunning(void) {
    return s_state == COUNTDOWN_RUNNING;
}

bool countdownIsComplete(void) {
    return s_state == COUNTDOWN_COMPLETE;
}

CountdownState countdownGetState(void) {
    return s_state;
}

unsigned long countdownGetRemainingMs(void) {
    if (s_state == COUNTDOWN_IDLE) {
        return COUNTDOWN_DURATION_MS;
    }
    if (s_state == COUNTDOWN_COMPLETE) {
        return 0;
    }
    unsigned long elapsed = millis() - s_startTimestamp;
    if (elapsed >= COUNTDOWN_DURATION_MS) {
        return 0;
    }
    return COUNTDOWN_DURATION_MS - elapsed;
}
