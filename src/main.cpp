#include <Arduino.h>
#include "pins.h"
#include "motors.h"
#include "line_sensors.h"
#include "ultrasonic.h"
#include "countdown.h"

void setup() {
    motorsInit();
    lineSensorsInit();
    ultrasonicInit();
    countdownInit();
}

void loop() {
    countdownUpdate();
    ultrasonicUpdate();
    lineSensorsRead();
    // Ticket 03 connects the combat FSM logic
}
