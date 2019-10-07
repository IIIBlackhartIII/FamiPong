/******************************************************************
 Created with PROGRAMINO IDE for Arduino - 27.09.2019 17:58:41
 Project     : FamiPong Domination  
 Author      : Jeffrey "Blackhart" Hepburn
 Description : Physical pong table for FamiLab's booth at MakerFaire Orlando 2019; paddles are driven by stepper motors on linear rails, scoring system run by IR break sensors.
******************************************************************/
#include <Wire.h>
#include <Adafruit_MotorShield.h>

#define DEBUG false

///STEPPER MOTORS///
//Motors are wired into the terminals Red, Yellow, (skip GND), Green, Grey (or Brown)
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); //create stepper motor object
Adafruit_StepperMotor *stepperMotor1 = AFMS.getStepper(200,1); //Player 1 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M1+M2)
Adafruit_StepperMotor *stepperMotor2 = AFMS.getStepper(200,2); //Player 2 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M3+M4)
/*
   Step types are "SINGLE", "DOUBLE", "INTERLEAVE", or "MICROSTEP":
      "Single" means single-coil activation
      "Double" means 2 coils are activated at once (for higher torque)
      "Interleave" means that it alternates between single and double to get twice the resolution (but of course its half the speed).
      "Microstep" is a method where the coils are PWM'd to create smooth motion between steps.
   */
#define stepType DOUBLE
#define numSteps 250
#define StepperSpeed 500 //set stepper motors speed in RPM

///CONTROLLERS///

/* TODO defining microswitch inputs to go here */


void setup()
{
  ///STEPPER MOTORS///
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  Wire.setClock(400000); //set speed of I2C bus, increases rate of polling to stepper sheild
  
  stepperMotor1->setSpeed(StepperSpeed); //set motor to run at defined speed
  stepperMotor2->setSpeed(StepperSpeed); //set motor to run at defined speed
}


void loop()
{

   ///STEPPER MOTORS///
   
   //Player1 Controller Input 
   moveStepper("Player1","Forward"); 
   delay(2000);
   moveStepper("Player1","Backward"); 
   delay(2000);
   
   //Player2 Controller Input
   //moveStepper("Player2","Forward"); 
   //moveStepper("Player2","Backward"); 
}


/////CUSTOM FUNCTIONS/////

void moveStepper(String playerMotor, String motorDir)
{
  if (playerMotor == "Player1"){
    if (motorDir == "Forward"){
      stepperMotor1->step(numSteps, FORWARD, stepType); //#Steps, Direction, Step Type
    }
    else{
      stepperMotor1->step(numSteps, BACKWARD, stepType); //#Steps, Direction, Step Type
    }
  }
  else{
    if (motorDir == "Forward"){
      stepperMotor2->step(numSteps, FORWARD, stepType); //#Steps, Direction, Step Type
    }
    else{
      stepperMotor2->step(numSteps, BACKWARD, stepType); //#Steps, Direction, Step Type
    }
  }
}
