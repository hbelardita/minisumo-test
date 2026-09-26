# 02: Sensor Acquisition and Countdown Sequencer

**What to build:** Asynchronous HC-SR04 ultrasonic echo capture using Pin Change Interrupts, analog TCRT5000 line sensing with border threshold detection, and a non-blocking 5-second countdown with 1 Hz LED blinking.

**Blocked by:** 01: TB6612FNG Motor Control and Pinout Setup

**Status:** ready-for-agent

- [ ] Line sensors read from `PIN_TCRT_LEFT=A2` and `PIN_TCRT_RIGHT=A3`, returning true for Border when analog reading < 400.
- [ ] Ultrasonic sensor triggers ping on `PIN_TRIG=11` every ~40 ms and captures echo pulse width on `PIN_ECHO=12` using `PCINT0_vect` without blocking the main loop.
- [ ] Opponent distances > 65 cm or timed-out pings are filtered out as no-target.
- [ ] Button on `PIN_BUTTON=10` initiates a non-blocking 5000 ms countdown.
- [ ] Status LED on `PIN_LED=13` toggles at 1 Hz during countdown and remains solid ON during combat.
