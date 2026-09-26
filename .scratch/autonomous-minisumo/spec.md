# Spec: Autonomous Mini-Sumo Robot

Status: ready-for-agent

## Problem Statement

Mini-Sumo competitors need an autonomous, reliable, and compliant robot that can compete in standard Mini-Sumo matches. Previous or naive Arduino implementations often suffer from:
1. Microcontroller brownouts and resets caused by motor back-EMF spikes when reversing direction near the ring edge.
2. Inability to detect the border in time because blocking ultrasonic rangefinder calls (such as standard `pulseIn`) stall the processor loop for up to 30 milliseconds.
3. Rigid or unresponsive maneuvering that drives off the ring or fails to track opponents dynamically.

The robot must operate within strict tournament rules (10 cm × 10 cm footprint, ≤ 500 g mass, 5-second countdown), reliably remain inside the Dohyo, locate the Opponent, and push them across the Border using TT gearmotors and an Arduino Nano.

## Solution

A modular, non-blocking firmware architecture running on an Arduino Nano (ATmega328P) and TB6612FNG motor driver. 

Key capabilities include:
- A non-blocking 5-second Countdown triggered by a tactile push-button with visual 1 Hz LED feedback before any motor actuation.
- Asynchronous Pin Change Interrupt (`PCINT0_vect`) ultrasonic echo capture for the front-center HC-SR04, preventing timing jitter and eliminating loop blocking.
- High-priority Border Avoidance triggered by two front analog Line Sensors reading the white Border, executing a sequence of brief 40 ms Coast (to protect against power-rail brownouts), followed by full reverse thrust (300 ms) and a directional pivot turn away from the detected border.
- Distance-proportional pursuit logic: Pivot Scan search at moderate rotational speed, regulated Approach at 60% throttle when an Opponent is detected between 30 cm and 65 cm, and maximum Attack thrust at 100% throttle when within 30 cm.
- Motor driver polarity mapping that accounts for the robot's physical wiring and explicit electrical coasting when velocity is zero.

## User Stories

1. As a competition operator, I want to initiate the robot's match sequence by pressing a physical push-button, so that the robot prepares for combat safely.
2. As a competition referee, I want the robot to remain completely stationary for exactly 5 seconds following the start signal, so that the match complies with the mandatory Countdown rules.
3. As a competition referee, I want the robot to flash its status LED at a steady 1 Hz cadence during the 5-second Countdown, so that all officials can visibly track the remaining safety pause.
4. As a competitor, I want the robot's status LED to remain solidly illuminated once the Countdown ends and active combat begins, so that its active state is clearly visible during the Match.
5. As a competitor, I want the robot to initiate an in-place Pivot Scan upon entering the Search state, so that it can sweep the Dohyo and locate the Opponent without blindly driving forward.
6. As a competitor, I want the robot to drive its Pivot Scan at a calibrated rotational speed (PWM ~130), so that the motor overcomes static gear friction while rotating slow enough for the ultrasonic sensor to sample acoustic echoes reliably.
7. As an embedded engineer, I want the HC-SR04 ultrasonic echo timing to be captured asynchronously via Pin Change Interrupts on Pin 12, so that the main control loop never blocks on `pulseIn()`.
8. As a competitor, I want the robot to filter out ultrasonic distance readings beyond 65 cm, so that spectators, referees, and objects outside the 77 cm Dohyo are ignored.
9. As a competitor, I want the robot to transition from Search to Approach when an Opponent is detected at a distance between 30 cm and 65 cm, so that it can align and close the distance under control.
10. As a competitor, I want the robot to throttle its motors to 60% PWM during Approach, so that it does not overshoot the target or destabilize its chassis before making contact.
11. As a competitor, I want the robot to transition from Approach to Attack when the Opponent is within 30 cm, so that it engages the Opponent with full torque and speed.
12. As a competitor, I want the robot to apply 100% PWM thrust during Attack, so that it exerts maximum pushing force to drive the Opponent across the Border.
13. As a competitor, I want the robot to revert from Approach or Attack back to Search if the Opponent echo is lost or exceeds 65 cm, so that it immediately resumes scanning rather than driving blindly.
14. As a competitor, I want the robot's two front Line Sensors to continuously sample the surface reflectance via analog ADC pins, so that any encounter with the white Border is detected instantly.
15. As a competitor, I want the robot to interpret an analog reflectance reading below 400 as the white Border and above 400 as the black Dohyo, so that lighting variations do not cause false border triggers.
16. As a competitor, I want Border Avoidance to immediately preempt and cancel any active Search, Approach, or Attack behavior, so that the robot avoids disqualification by driving out of the ring.
17. As an embedded engineer, I want the robot to execute a 40 ms Coast immediately upon detecting the Border before applying reverse power, so that TT motor back-EMF does not cause a brownout reset on the Arduino Nano 5V rail.
18. As a competitor, I want the robot to execute a 300 ms full-power reverse thrust following the Coast phase of Border Avoidance, so that the chassis backs completely clear of the 5 cm white Border.
19. As a competitor, I want the robot to pivot right for 220 ms if only the left Line Sensor detected the Border, so that it steers back toward the interior of the Dohyo.
20. As a competitor, I want the robot to pivot left for 220 ms if only the right Line Sensor detected the Border, so that it steers back toward the interior of the Dohyo.
21. As a competitor, I want the robot to spin 180 degrees for 220 ms if both Line Sensors detected the Border simultaneously, so that it turns completely around and re-enters the Dohyo.
22. As a competitor, I want the robot to transition smoothly to Search once the Border Avoidance pivot completes, so that it immediately resumes looking for the Opponent.
23. As a hardware builder, I want the motor driver control layer to map negative speed values to forward motion on Channel A and Channel B as required by my physical wiring, so that directional logic in firmware is clean and intuitive.
24. As an embedded developer, I want the motor driver layer to place the TB6612FNG in true high-impedance Coast mode when speed is 0 or when `motorsCoast()` is invoked, so that motors do not suffer unintentional short-circuit braking.
25. As a competition operator, I want the match to be halted by cutting main battery power via the hardware power switch, so that firmware does not waste cycles polling for stop buttons or risk false contact-triggered aborts during a Match.

## Implementation Decisions

- **Direct Firmware Architecture**:
  The firmware is organized cleanly directly for the Arduino Nano ATmega328P target:
  - Global state and hardware pin mappings aligned with the user's circuit.
  - Direct hardware driver routines for TB6612FNG, TCRT5000, and HC-SR04.
  - Cooperative non-blocking main loop executing sensor reads, deterministic state machine transitions, and actuator updates.

- **TB6612FNG Pin Assignment & Polarity (Yellow TT Motors)**:
  - `PIN_STBY = 8`
  - `PIN_AIN1 = 7`, `PIN_AIN2 = 9`, `PIN_PWMA = 5` (Right Motor, Channel A)
  - `PIN_BIN1 = 4`, `PIN_BIN2 = 2`, `PIN_PWMB = 6` (Left Motor, Channel B)
  - Physical polarity inversion: Forward motion corresponds to `AIN1=LOW, AIN2=HIGH` and `BIN1=LOW, BIN2=HIGH`.
  - Coasting: To achieve true coast on TB6612FNG when speed is 0 or during the initial phase of Border Avoidance, both direction pins are pulled LOW (`IN1=LOW, IN2=LOW`) or `PIN_STBY` is brought LOW, avoiding short-circuit regenerative braking.

- **Asynchronous Ultrasonic Capture (HC-SR04)**:
  - `PIN_TRIG = 11`, `PIN_ECHO = 12`
  - Trigger pulses (10 µs) are dispatched periodically every 40 ms via a non-blocking `millis()` / `micros()` scheduler.
  - Echo pulse width is captured by an interrupt service routine on Pin Change Interrupt 0 (`PCINT0_vect` covering `PCINT4` / Pin 12).
  - Out-of-bounds readings (> 65 cm) and timeouts are mapped to `NO_OPPONENT`.
  - Documented in [ADR-0002](file:///home/horacio/Documents/tmp/minisumo-nano/docs/adr/0002-async-ultrasonic-interrupts.md).

- **Line Sensors & Thresholds (TCRT5000)**:
  - `PIN_TCRT_LEFT = A2`, `PIN_TCRT_RIGHT = A3`
  - Calibrated threshold: `BORDER_THRESHOLD = 400` with hysteresis. Values `< 400` signify the white Border; values `>= 400` signify the black Dohyo surface.

- **Border Avoidance Timing (ADR-0001)**:
  - Coast Phase: 40 ms duration with motors in high-impedance mode to dissipate inductive energy.
  - Reverse Phase: 300 ms straight backward at 100% PWM (`-255, -255`).
  - Pivot Phase: 220 ms at 80% PWM (`200`) turning away from the trigger (Left sensor -> Turn Right; Right sensor -> Turn Left; Both -> Spin 180°).
  - Documented in [ADR-0001](file:///home/horacio/Documents/tmp/minisumo-nano/docs/adr/0001-coast-before-reverse.md).

- **Tactical Speed Profiles**:
  - `Pivot Scan`: PWM ±130.
  - `Approach`: PWM +150 (60%).
  - `Attack`: PWM +255 (100%).

## Testing Decisions

- **Build and Physical Target Verification**:
  - Firmware builds cleanly with zero warnings under PlatformIO for `env:nanoatmega328`.
  - Serial logging mode optional during bench testing to verify sensor readouts and state transitions in real time.
  - On-target validation of countdown timing, HC-SR04 echo responsiveness, and line detection threshold.

## Out of Scope

- Remote radio/IR referee modules (reserved for future iterations; current trigger is onboard push-button).
- Dynamic sensor auto-calibration routines on startup (fixed calibrated threshold `< 400` used for simplicity and reliability).
- Rear line sensors or physical microswitch bumpers (robot uses 2x front TCRT5000 and 1x front HC-SR04).
- Dynamic speed PID controllers or wheel encoder feedback (open-loop PWM control suited for TT gearmotors).

## Further Notes

- All code must build cleanly under PlatformIO for `env:nanoatmega328` (`atmelavr` platform).
- Prescaler registers for Timer0 must remain untouched to preserve Arduino `millis()` and `micros()` clock integrity.
