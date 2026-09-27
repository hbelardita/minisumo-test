#include <Arduino.h>
#include "pins.h"
#include "motors.h"
#include "line_sensors.h"
#include "ultrasonic.h"
#include "countdown.h"
#include "fsm.h"

void setup() {
    motorsInit();
    lineSensorsInit();
    ultrasonicInit();
    countdownInit();
    fsmInit();
}

void loop() {
    countdownUpdate();
    ultrasonicUpdate();
    fsmUpdate();
}
