# FamiPong
Physical pong table for FamiLab's booth at MakerFaire Orlando 2019;
System controlled by an Arduino Mega 2560 with the Adafruit Stepper Motor Shield V2. 

Paddles are driven by NEMA17 stepper motors on linear rails, scoring system run by IR break sensors.
Points scored are displayed on a Neopixel LED strip, and score delta between the players on a 5V voltimeter. 

A pair of LM2596 Buck Converters are used to step the voltage down.
12v 6A power supply is used as input, which is fed directly out for the solenoids, and is given to the stepper shield.
One buck converter steps down the 12v to 9v for the Arduino, to help with hardware longevity and heat.
One buck converter steps down the 12v to 5v for the IR break beams and Neopixel strip. 

The stepper motors are driven via the AccelStepper library (http://www.airspayce.com/mikem/arduino/AccelStepper/index.html), wrapped around a customised version of the Adafruit Stepper Shield library (https://github.com/IIIBlackhartIII/Adafruit_StepperV2_PerformanceBranch)

