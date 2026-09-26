# 03: Combat State Machine and Border Avoidance Reflex

**What to build:** Deterministic cooperative Finite State Machine implementing idle waiting, 5-second countdown, search pivot scan, distance-proportional attack, and priority border avoidance reflex.

**Blocked by:** 02: Sensor Acquisition and Countdown Sequencer

**Status:** ready-for-agent

- [ ] Finite State Machine handles states: `STATE_IDLE`, `STATE_COUNTDOWN`, `STATE_SEARCH`, `STATE_APPROACH`, `STATE_ATTACK`, `STATE_BORDER_AVOID`.
- [ ] In `STATE_SEARCH`, robot executes in-place Pivot Scan at PWM ~130.
- [ ] In `STATE_APPROACH`, robot moves forward at 60% PWM (150) when opponent is between 30 cm and 65 cm.
- [ ] In `STATE_ATTACK`, robot drives full forward at 100% PWM (255) when opponent is < 30 cm.
- [ ] Whenever either TCRT5000 detects white Border (< 400), `STATE_BORDER_AVOID` immediately preempts current behavior.
- [ ] `STATE_BORDER_AVOID` executes non-blocking 40 ms Coast -> 300 ms Reverse -> 220 ms Pivot Turn -> Return to `STATE_SEARCH`.
- [ ] Firmware compiles cleanly with `pio run` for `env:nanoatmega328`.
