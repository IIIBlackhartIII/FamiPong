/******************************************************************
 Created with PROGRAMINO IDE for Arduino - 27.09.2019 17:58:41
 Project     : FamiPong Domination  
 Author      : Jeffrey "Blackhart" Hepburn
 Description : Physical pong table for FamiLab's booth at MakerFaire Orlando 2019; paddles are driven by stepper motors on linear rails, scoring system run by IR break sensors.
 Libraries: Adafruit Motor Shield V2 (https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library); AccelStepper (http://www.airspayce.com/mikem/arduino/AccelStepper/index.html)
******************************************************************/
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>

#define DEBUG false


///STEPPER MOTORS///
/******************************************************************/
//NEMA 17 Stepper Motors, 200 Steps per Revolution

//Motors are wired into the terminals Red, Yellow, (skip GND), Green, Grey (or Brown)
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); //create stepper motor object
Adafruit_StepperMotor *stepperMotor1 = AFMS.getStepper(200,1); //Player 1 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M1+M2)
Adafruit_StepperMotor *stepperMotor2 = AFMS.getStepper(200,2); //Player 2 Nema17 motor, 200 steps per revolution, connected to Port1 (terminals M3+M4)
/*
   -Step types are "SINGLE", "DOUBLE", "INTERLEAVE", or "MICROSTEP":
      -"Single" means single-coil activation
      -"Double" means 2 coils are activated at once (for higher torque)
      -"Interleave" means that it alternates between single and double to get twice the resolution (but of course its half the speed).
      -"Microstep" is a method where the coils are PWM'd to create smooth motion between steps.
*/
#define stepType DOUBLE
#define StepperSpeed 3500 //set stepper motors speed in RPM
#define StepperAccel 10000 //set stepper motors acceleration in steps per second per second
#define PaddleDist 7500 //movement target for left/right

//Define Single Step Functions for AccelStepper Use
void forwardStepP1()
{
  stepperMotor1->onestep(FORWARD,stepType);
}
void backwardStepP1()
{
  stepperMotor1->onestep(BACKWARD,stepType);
}
void forwardStepP2()
{
  stepperMotor2->onestep(FORWARD,stepType);
}
void backwardStepP2()
{
  stepperMotor2->onestep(BACKWARD,stepType);
}

//Define Stepper Motors as AccelStepper Objects
AccelStepper Astepper1(forwardStepP1, backwardStepP1); // use functions to step
AccelStepper Astepper2(forwardStepP2, backwardStepP2); // use functions to step

// Motor check startup sequence variables
bool BootSequenceTestRun = false;
int MovementChecked = 0;
/******************************************************************/



///JOYSTICKS///
/******************************************************************/
//Adafruit Small Microswitch Joystick (Sanwa type)

//Pinout (1-5), with the plug facing the top of the joystick, is Left,Right,Up,Down,GND
//On our version of the joystick this is colour coded Red,Orange,Yellow,Green,Black

//With our Joysticks mounted sideways, Left/Right will translate to Up/Down

//Digital Pins
#define Player1Up 22  //Player 1 UP (Yellow) 
#define Player1Down 23  //Player 1 DOWN (Green)
#define Player1Left 24  //Player 1 LEFT (Red)  
#define Player1Right 25  //Player 1 RIGHT (Orange) 

#define Player2Up 52  //Player 2 UP (Yellow) 
#define Player2Down 53  //Player 2 DOWN (Green)
#define Player2Left 50  //Player 2 LEFT (Red)  
#define Player2Right 51  //Player 2 RIGHT (Orange) 

/******************************************************************/


/* TODO adding end stop limit switch inputs to go here*/

///IR BREAK BEAM SENSORS///
/******************************************************************/
//Break Beams are mounted at the end of the board, after the solenoid rails, and are used to detect the ball falling into the trough

//Digital Pins
#define Player1SensorPin 26 //Player 1 Break Beam 
#define Player2SensorPin 48 //Player 2 Break Beam

//Scoring State Variables
bool canScore = true;

int P1SensorState = 0, P2SensorState = 0;

unsigned long resetTimer = 0; //when was the score last triggered
#define resetDelayPeriod 5000; //how many milliseconds does the ball need to be taken out for before someone can score again)

//Player Score Variables
int Player1Score = 0, Player2Score = 0;
/******************************************************************/

/*TODO Adding Neopixels to go here*/

/*TODO Adding Voltmeter to go here*/


void setup()
{
  ///STEPPER MOTORS///
  Serial.begin(19200); // set up Serial library at 19200 bps
  Serial.println("Stepper test!");
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  Wire.setClock(400000); //set speed of I2C bus, increases rate of polling to stepper sheild
  
  Astepper1.setMaxSpeed(StepperSpeed); //set max speed on Motor1
  Astepper1.setAcceleration(StepperAccel); //set acceleration on Motor1
  
  Astepper2.setMaxSpeed(StepperSpeed); //set max speed on Motor2
  Astepper2.setAcceleration(StepperAccel); //set acceleration on Motor2
  
  //Prime test sequence
  if (BootSequenceTestRun == false){
    //Test Movement Distance
    Astepper1.moveTo(600);
    Astepper2.moveTo(600);
  }
  
  ///JOYSTICKS///
  pinMode(Player1Up,INPUT_PULLUP);
  pinMode(Player1Down,INPUT_PULLUP);
  
  pinMode(Player2Up,INPUT_PULLUP);
  pinMode(Player2Down,INPUT_PULLUP);
  
  ///IR SENSORS///
  pinMode(Player1SensorPin, INPUT); //init the sensor pin as input
  digitalWrite(Player1SensorPin, HIGH); //turn on the pullup
  
  pinMode(Player2SensorPin, INPUT);  //init the sensor pin as input
  digitalWrite(Player2SensorPin, HIGH); //turn on the pullup
}


void loop()
{
  ///MOTORS MOVEMENT///
  /******************************************************************/
  if (BootSequenceTestRun == false){
    TestMotorsMovement();
  } 
  
  if (BootSequenceTestRun == true){
    //Player1 Controller Input
    //If only 1 direction is triggered, move that direction, if both released or both depressed, stop moving
    if (digitalRead(Player1Up) == LOW && digitalRead(Player1Down) == HIGH){
      Astepper1.moveTo(PaddleDist);
    } 
    else if (digitalRead(Player1Down) == LOW && digitalRead(Player1Up) == HIGH){
      Astepper1.moveTo(-PaddleDist);
    }
    else {
      Astepper1.moveTo(Astepper1.currentPosition());
    }


    //Player2 Controller Input
    //If only 1 direction is triggered, move that direction, if both released or both depressed, stop moving
    if (digitalRead(Player2Up) == LOW && digitalRead(Player2Down) == HIGH){
      Astepper2.moveTo(PaddleDist);
    } 
    else if (digitalRead(Player2Down) == LOW && digitalRead(Player2Up) == HIGH){
      Astepper2.moveTo(-PaddleDist);
    }
    else {
      Astepper2.moveTo(Astepper2.currentPosition());
    }
  }

  //RUN EVERY LOOP, commands stepper motors to update//
  Astepper1.run();
  Astepper2.run();
  /******************************************************************/
  
  
  ///SCORING///
  /******************************************************************/
  CheckScoring();
  
  

  
  /******************************************************************/
}

/******************************************************************/
/////CUSTOM FUNCTIONS/////
/******************************************************************/
void CheckScoring()
{
  //check sensors states
  P1SensorState = digitalRead(Player1SensorPin); 
  P2SensorState = digitalRead(Player2SensorPin);
  
  //check if a player scored, and can score
  if (P1SensorState == LOW && canScore == true){
    Player1Score += 1;
    Serial.println("Player 1 Scored!");
    Serial.println("Player 1's Score is now: " + Player1Score);
    
    canScore = false;
  }
  
  if (P2SensorState == LOW && canScore == true){
    Player2Score += 1;
    Serial.println("Player 2 Scored!");
    Serial.println("Player 2's Score is now: " + Player2Score);
    
    canScore = false;
  }
  
  //once ball is removed, scoring is still locked. Set the reset timer to delay scoring
  if (P1SensorState == HIGH && P2SensorState == HIGH && canScore == false){
    resetTimer = millis();
  }
  
  //once the current time is greater than the resetTime + the delay (how many seconds before its fair to score again) unlock scoring again
  if (millis() > resetTimer + resetDelayPeriod){
    canScore = true;
  }
}

void TestMotorsMovement()
{
  if(Astepper1.distanceToGo() <= 0)
    Astepper1.moveTo(-Astepper1.currentPosition());
    MovementChecked += 1;
  if(Astepper2.distanceToGo() <= 0)
    Astepper2.moveTo(-Astepper2.currentPosition());
    MovementChecked += 1;
    
  if (MovementChecked >= 4){
    BootSequenceTestRun = true;
  }
}
