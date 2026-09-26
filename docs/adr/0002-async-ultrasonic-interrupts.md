# Asynchronous Ultrasonic Capture via Pin Change Interrupt

Standard Arduino ultrasonic routines rely on `pulseIn()`, which blocks microcontroller execution for up to 30 ms per ping and risks missing Border crossings at speed. We capture HC-SR04 echo pulse duration asynchronously via native ATmega328P Pin Change Interrupts (`PCINT0_vect` on Pin 12), ensuring the primary FSM and optical line sensor checks execute with zero latency jitter.
