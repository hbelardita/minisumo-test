# Mini-Sumo

Autonomous robotic competitor designed to locate and push an opposing robot out of a circular ring following standard mini-sumo competition regulations.

## Language

**Dohyo**:
The circular competition ring having a flat black surface and a contrasting perimeter.
_Avoid_: Arena, pitch, field, table

**Border**:
The 5 cm white outer band marking the boundary of the Dohyo beyond which a robot is disqualified or defeated upon contact with the outside floor.
_Avoid_: Line, edge, margin, boundary

**Opponent**:
The opposing autonomous robot competing inside the Dohyo.
_Avoid_: Enemy, target, adversary, obstacle

**Countdown**:
The mandatory 5-second safety and preparation pause between the start signal and any motor movement.
_Avoid_: Delay, wait-time, start-pause

**Match**:
A competitive bout between two autonomous robots on the Dohyo.
_Avoid_: Round, battle, fight, game

**Line Sensor**:
A downward-facing optical reflectance sensor positioned beneath the chassis to distinguish the black Dohyo surface from the white Border.
_Avoid_: Floor sensor, edge detector, ground sensor

**Search**:
The tactical behavior of scanning the Dohyo to locate the Opponent when no target is detected.
_Avoid_: Scan, wander, hunt, patrol

**Approach**:
The forward pursuit behavior toward a detected Opponent at regulated speed before committing to full contact.
_Avoid_: Chase, advance, stalk

**Attack**:
The offensive behavior of accelerating directly toward the detected Opponent to push them across the Border.
_Avoid_: Charge, ram, strike, rush

**Border Avoidance**:
The defensive reflex executed when a Line Sensor detects the Border to halt movement toward the edge and steer back onto the Dohyo.
_Avoid_: Edge recovery, line escape, retreat

**Pivot Scan**:
The stationary rotational movement executed during Search to sweep the Proximity Sensor across the Dohyo.
_Avoid_: Spin search, pirouette, turn scan

