# 01: TB6612FNG Motor Control and Pinout Setup

**What to build:** Pin definitions and motor control driver routines matching the physical wiring of the robot, providing forward/backward throttle, active braking, and high-impedance coasting.

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

- [x] Pin definitions configured for TB6612FNG (`PIN_STBY=8`, `PIN_AIN1=7`, `PIN_AIN2=9`, `PIN_PWMA=5`, `PIN_BIN1=4`, `PIN_BIN2=2`, `PIN_PWMB=6`).
- [x] Speed control function `motorsSetSpeed(leftSpeed, rightSpeed)` maps negative speed to forward motion according to physical wiring.
- [x] Explicit coasting function `motorsCoast()` places driver in high-impedance mode when speed is 0 or during border coasting as defined in ADR-0001.
- [x] Active braking function `motorsBrake()` shorts motor terminals for fast stopping.
