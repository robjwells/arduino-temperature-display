## Arduino 7-segment temperature display

This sketch reads the temperature from a TMP36, and displays it on three
7-segment LEDs, via three shift registers.

It uses the `INTERNAL` voltage (about 1.1V) as the analog reference, as the
TMP36 only reaches 1V at about 50°C, and I'm using this as a basic room
thermometer.

The `INTERNAL` voltage on an Atmega 328P isn't exactly 1.1V, so you'll want to 
[measure the voltage on your particular chip][measure] and substitute in the
value in millivolts.

[measure]: https://forum.arduino.cc/t/measurement-of-bandgap-voltage/38215/4

You could use the default analog reference (5V), but then you'll only be able
to measure in roughly 0.5°C increments.

When I have time, I'll add a wiring diagram and some additional details.
