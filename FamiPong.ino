/******************************************************************
 Created with PROGRAMINO IDE for Arduino - 27.09.2019 17:58:41
 Project     : FamiPong Domination  
 Author      : Jeffrey "Blackhart" Hepbun
 Description : Physical pong table for FamiLab's booth at MakerFaire Orlando 2019; paddles are driven by stepper motors on linear rails, scoring system run by IR break sensors.
******************************************************************/
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <utility/Adafruit_PWMServoDriver.h>

#define DEBUG false

///STEPPER MOTORS//
//Motors are wired into the terminals Red, Yellow, (skip GND), Green, Grey (or Brown)
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); //create stepper motor object
int StepperSpeed = 50; //set stepper motors speed in RPM

void setup()
{
    ///STEPPER MOTORS//
    Adafruit_StepperMotor stepperMotor1 = AFMS.getStepper(200,1); //Player 1 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M1+M2)
    Adafruit_StepperMotor stepperMotor2 = AFMS.getStepper(200,2); //Player 2 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M3+M4)
    stepperMotor1.setSpeed(StepperSpeed); //set motor to run at defined speed
    stepperMotor2.setSpeed(StepperSpeed); //set motor to run at defined speed
}

void loop()
{

   ///STEPPER MOTORS///
   /*
   Step types are "SINGLE", "DOUBLE", "INTERLEAVE", or "MICROSTEP":
      "Single" means single-coil activation
      "Double" means 2 coils are activated at once (for higher torque)
      "Interleave" means that it alternates between single and double to get twice the resolution (but of course its half the speed).
      "Microstepping" is a method where the coils are PWM'd to create smooth motion between steps.
   */
   stepperMotor1.step(100, FORWARD, DOUBLE); //#Steps, Direction, Step Type
   stepperMotor2.step(100, FORWARD, DOUBLE); //#Steps, Direction, Step Type
   
   delay(1000);
}
