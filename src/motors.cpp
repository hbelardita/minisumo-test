#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "motors.h"

static void setChannelSpeed(uint8_t in1Pin, uint8_t in2Pin, uint8_t pwmPin, int speed) {
    if (speed == 0) {
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, LOW);
        analogWrite(pwmPin, 0);
        return;
    }

    // In physical wiring, inverted polarity maps to forward motion
    int invertedSpeed = -speed;
    int pwmDuty = 0;

    if (invertedSpeed > 0) {
        digitalWrite(in1Pin, HIGH);
        digitalWrite(in2Pin, LOW);
        pwmDuty = invertedSpeed;
    } else {
        digitalWrite(in1Pin, LOW);
        digitalWrite(in2Pin, HIGH);
        pwmDuty = -invertedSpeed;
    }

    if (pwmDuty > 255) {
        pwmDuty = 255;
    }

    analogWrite(pwmPin, pwmDuty);
}

void motorsInit(void) {
    pinMode(PIN_STBY, OUTPUT);
    pinMode(PIN_AIN1, OUTPUT);
    pinMode(PIN_AIN2, OUTPUT);
    pinMode(PIN_PWMA, OUTPUT);
    pinMode(PIN_BIN1, OUTPUT);
    pinMode(PIN_BIN2, OUTPUT);
    pinMode(PIN_PWMB, OUTPUT);

    motorsCoast();
}

void motorsCoast(void) {
    digitalWrite(PIN_STBY, LOW);
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
    analogWrite(PIN_PWMA, 0);
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, LOW);
    analogWrite(PIN_PWMB, 0);
}

void motorsBrake(void) {
    digitalWrite(PIN_STBY, HIGH);
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, HIGH);
    analogWrite(PIN_PWMA, 255);
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, HIGH);
    analogWrite(PIN_PWMB, 255);
}

void motorsSetSpeed(int leftSpeed, int rightSpeed) {
    if (leftSpeed == 0 && rightSpeed == 0) {
        motorsCoast();
        return;
    }

    digitalWrite(PIN_STBY, HIGH);

    // Channel A: Right Motor
    setChannelSpeed(PIN_AIN1, PIN_AIN2, PIN_PWMA, rightSpeed);

    // Channel B: Left Motor
    setChannelSpeed(PIN_BIN1, PIN_BIN2, PIN_PWMB, leftSpeed);
}
