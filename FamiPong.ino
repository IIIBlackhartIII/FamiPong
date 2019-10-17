/******************************************************************
 Created with PROGRAMINO IDE for Arduino - 27.09.2019 17:58:41
 Project     : FamiPong Domination  
 Author      : Jeffrey "Blackhart" Hepburn
 Description : Physical pong table for FamiLab's booth at MakerFaire Orlando 2019; paddles are driven by stepper motors on linear rails, scoring system run by IR break sensors.
 Libraries: -Adafruit Motor Shield V2 Performance Branch (https://github.com/IIIBlackhartIII/Adafruit_StepperV2_PerformanceBranch); 
            -AccelStepper (http://www.airspayce.com/mikem/arduino/AccelStepper/index.html);
            -Adafruit Neopixel (https://github.com/adafruit/Adafruit_NeoPixel);
******************************************************************/
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include <Adafruit_NeoPixel.h>


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
#define PaddleDist 10 //movement target for left/right
bool Stepper1IsMoving = false; //keeps track of whether the stepper motor is moving or stopped
bool Stepper2IsMoving = false; //keeps track of whether the stepper motor is moving or stopped

//Define Step Functions for AccelStepper Use
void forwardStepP1()
{
  stepperMotor1->quickstep(FORWARD);
}
void backwardStepP1()
{
  stepperMotor1->quickstep(BACKWARD);
}
void forwardStepP2()
{
  stepperMotor2->quickstep(FORWARD);
}
void backwardStepP2()
{
  stepperMotor2->quickstep(BACKWARD);
}

//Define Stepper Motors as AccelStepper Objects
AccelStepper Astepper1(forwardStepP1, backwardStepP1); // use functions to step
AccelStepper Astepper2(forwardStepP2, backwardStepP2); // use functions to step

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

unsigned long resetTimer = 0; // timestamp when was the score last triggered
bool resetWaiting  = false; //used to make sure the timer is only set once when ball removed
const long resetDelay = 1500; //how many milliseconds does the ball need to be taken out for before someone can score again

//Player Score Variables
int Player1Score = 0, Player2Score = 0;
/******************************************************************/

///NEOPIXELS///
/******************************************************************/
#define LED_COUNT 60 //how many LEDs on strip
#define LED_Brightness 50 //how bright the LED is
#define fadeAmount 3 //how many points to fade the LED by
#define Pixel_Pin 36 //NeoPixel strip data pin

Adafruit_NeoPixel strip(LED_COUNT, Pixel_Pin, NEO_GRB + NEO_KHZ800); //declare strip
/******************************************************************/

#define VoltDisplay 45 //define PWM pin for the Voltmeter
int DisplayScore = 0;


void setup()
{
  ///STEPPER MOTORS///
  Serial.begin(115200); // set up Serial library at 115200 bps
  Serial.println("Stepper test!");
  
  AFMS.begin();  // create with the default frequency 1.6KHz
  Wire.setClock(500000L); //set speed of I2C bus, increases rate of polling to stepper sheild
  
  Astepper1.setSpeed(StepperSpeed); //set stepper const speed on Motor1
  Astepper1.setMaxSpeed(StepperSpeed); //set max speed on Motor1
  Astepper1.setAcceleration(StepperAccel); //set acceleration on Motor1
  
  Astepper2.setSpeed(StepperSpeed); //set stepper const speed on Motor2
  Astepper2.setMaxSpeed(StepperSpeed); //set max speed on Motor2
  Astepper2.setAcceleration(StepperAccel); //set acceleration on Motor2
  
  //INIT PWM's on Motors to 100%
  stepperMotor1->onestep(FORWARD,stepType);
  stepperMotor1->release();
  stepperMotor2->onestep(FORWARD,stepType);
  stepperMotor2->release();
  
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
  
  ///DISPLAYS///
  pinMode(VoltDisplay,OUTPUT); //set Voltmeter pin to ouput
  
  strip.begin(); //INIT Neopixel strip object
  strip.show(); //Turn off all pixels
  strip.setBrightness(LED_Brightness); //set max brightness of Neopixel LEDs
}


void loop()
{
  ///MOTORS MOVEMENT///
  /******************************************************************/
  //Player1 Controller Input
  //If only 1 direction is triggered, move that direction, if both released or both depressed, stop moving
  if (digitalRead(Player1Up) == LOW && digitalRead(Player1Down) == HIGH){
    Astepper1.move(PaddleDist);
    Stepper1IsMoving = true;
  } 
  else if (digitalRead(Player1Down) == LOW && digitalRead(Player1Up) == HIGH){
    Astepper1.move(-PaddleDist);
    Stepper1IsMoving = true;
  }
  else {
    if (Stepper1IsMoving){
      brakeStepper(1);
    }
  }


  //Player2 Controller Input
  //If only 1 direction is triggered, move that direction, if both released or both depressed, stop moving
  if (digitalRead(Player2Up) == LOW && digitalRead(Player2Down) == HIGH){
    Astepper2.move(PaddleDist);
    Stepper2IsMoving = true;
  } 
  else if (digitalRead(Player2Down) == LOW && digitalRead(Player2Up) == HIGH){
    Astepper2.move(-PaddleDist);
    Stepper2IsMoving = true;
  }
  else {
    if (Stepper2IsMoving){
      brakeStepper(2);
    }
  }

  //RUN EVERY LOOP, commands stepper motors to update//
  if (Stepper1IsMoving){
    Astepper1.runSpeed();
  }
  if (Stepper2IsMoving){
    Astepper2.runSpeed();
  }
  /******************************************************************/
  
  
  ///SCORING///
  /******************************************************************/
  CheckScoring();
  VoltScoreDisplay();
  
  
  //Check Victory Condition
  if (abs(Player1Score - Player2Score) >= 3){
    if (Player1Score > Player2Score){
      Serial.println("Player 1 Wins!");
      Serial.println("Player 1's Final Score Was: " + Player1Score);
      VictoryDisplay(1);
    }
    else {
      Serial.println("Player 2 Wins!");
      Serial.println("Player 2's Final Score Was: " + Player2Score);
      VictoryDisplay(2);
    }
    
    Player1Score = 0;
    Player2Score = 0;
  }
  /******************************************************************/
}

/******************************************************************/
/////CUSTOM FUNCTIONS/////
/******************************************************************/
void brakeStepper(int team)
{
  if (team == 1){
    Astepper1.stop();
    stepperMotor1->release(); //release motor when not moving to prevent over heating
    Stepper1IsMoving = false;
  }
  else{
    Astepper2.stop();
    stepperMotor2->release(); //release motor when not moving to prevent over heating
    Stepper2IsMoving = false;
  }
}

void CheckScoring()
{
  //check sensors states
  P1SensorState = digitalRead(Player1SensorPin); 
  P2SensorState = digitalRead(Player2SensorPin);
  
  //check if a player scored, and can score
  //sensor of P1 gives P2 points, because P2 got the ball past P1's paddle, and vice versa
  if (P1SensorState == LOW && canScore == true){
    Player2Score += 1;
    Serial.println("Player 2 Scored!");
    Serial.println("Player 2's Score is now: " + Player2Score);
    ScorePointDisplay(2);
    
    canScore = false;
  }
  else if (P2SensorState == LOW && canScore == true){
    Player1Score += 1;
    Serial.println("Player 1 Scored!");
    Serial.println("Player 1's Score is now: " + Player1Score);
    ScorePointDisplay(1);
    
    canScore = false;
  }
  
  //once ball is removed, scoring is still locked. Set the reset timer to delay scoring
  if (P1SensorState == HIGH && P2SensorState == HIGH && canScore == false){
    if (resetWaiting == false){
      resetTimer = millis();
      resetWaiting = true;
    }
  }
  
  //once the current time is greater than the resetTime + the delay (how many seconds before its fair to score again) unlock scoring again

  if (millis() - resetTimer >= resetDelay && resetWaiting == true){
    canScore = true; //allow scoring again
    resetWaiting = false; //allow reset timer to be set next time
    strip.clear(); //reset neopixels
    strip.show(); //show cleared pixels
  }
}

void ScorePointDisplay(int team)
{
  int i = 0;
  strip.clear();
  
  if (team == 1){
    for (i; i < LED_COUNT/2; i++){
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      strip.setPixelColor(i, strip.Color(0, 150, 0));
      strip.show();   // Send the updated pixel colors to the hardware.
    }
  }
  else {
    i = LED_COUNT/2;
    for(i; i<LED_COUNT; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright red color:
      strip.setPixelColor(i, strip.Color(150, 0, 0));
      strip.show();   // Send the updated pixel colors to the hardware.
    }
  }
}

void VoltScoreDisplay()
{
  DisplayScore = (Player1Score - Player2Score) + 3;
  
  //10Hz asynchronous update rate on Voltmeter
  if (millis() % 100 == 0){
    analogWrite(VoltDisplay, 153*(0.166667*DisplayScore));
  }
}

void VictoryDisplay (int team)
{
  int upperLimit = LED_COUNT; 
  int lowerLimit = 0;
  if (team == 1){
    upperLimit = LED_COUNT/2;
  }
  else {
    lowerLimit = LED_COUNT/2;
  }
  
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i= lowerLimit; i<upperLimit; i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
  }
}