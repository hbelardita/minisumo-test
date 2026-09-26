#if defined(UNIT_TEST) || !defined(ARDUINO)
#include "mock_arduino.h"
#else
#include <Arduino.h>
#endif
#include "pins.h"
#include "motors.h"

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
    // In physical wiring, negative speed maps to forward motion
    int speedA = -rightSpeed;
    if (speedA > 0) {
        digitalWrite(PIN_AIN1, HIGH);
        digitalWrite(PIN_AIN2, LOW);
    } else if (speedA < 0) {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, HIGH);
        speedA = -speedA;
    } else {
        digitalWrite(PIN_AIN1, LOW);
        digitalWrite(PIN_AIN2, LOW);
        speedA = 0;
    }

    // Channel B: Left Motor
    // In physical wiring, negative speed maps to forward motion
    int speedB = -leftSpeed;
    if (speedB > 0) {
        digitalWrite(PIN_BIN1, HIGH);
        digitalWrite(PIN_BIN2, LOW);
    } else if (speedB < 0) {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, HIGH);
        speedB = -speedB;
    } else {
        digitalWrite(PIN_BIN1, LOW);
        digitalWrite(PIN_BIN2, LOW);
        speedB = 0;
    }

    if (speedA > 255) speedA = 255;
    if (speedB > 255) speedB = 255;

    analogWrite(PIN_PWMA, speedA);
    analogWrite(PIN_PWMB, speedB);
}
