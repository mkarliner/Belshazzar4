
/* 
 Stepper Motor Control - one step at a time
 
 This program drives a unipolar or bipolar stepper motor. 
 The motor is attached to digital pins 8 - 11 of the Arduino.
 
 The motor will step one step at a time, very slowly.  You can use this to
 test that you've got the four wires of your stepper wired to the correct
 pins. If wired correctly, all steps should be in the same direction.BelshazzarBelshazzarBelshazzarBelshazzarBelshazzarBelshazzarBelshazzar
 
 Use this also to count the number of steps per revolution of your motor,
 if you don't know it.  Then plug that number into the oneRevolution
 example to see if you got it right.
 
 Created 30 Nov. 2009
 by Tom Igoe
 
 */

#include "Tlc5940.h"
#include <stdarg.h>

//const int stepsPerRevolution = 48*8;  // change this to fit the number of steps per revolution
// for your motor
const int homeSwitch = 8;
const int stepPin = 7;
const int dirPin = 6;


int moving = 0;

void p(char *fmt, ... ){
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

void returnToHome() {

  while(!digitalRead(homeSwitch)) {
    step(0,1,1000);
  }
  delay(500);
  // Move forward to allow for leeway on return journey
  step(1,200,1000);
}




void setAll(int val) {
  int l;
  Tlc.clear();
  for(l=0; l<16; l++) {
    Tlc.set(l, val);
  }
  Tlc.update();
}

void step(int dir, int steps, int delay) 
{
  int x;

  digitalWrite(dirPin, dir);
  for(x=0; x< steps; x++) {
    digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
    //delayMicroseconds(000);
    digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
    digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
    delayMicroseconds(delay); 
  }
}

void move(int steps) 
{
   moving = steps;
}


int stepCount = 0;         // number of steps the motor has taken
String inputString="";
int stringComplete = false;

void setup() {
  const int pre_load_count=10;
  int c;

  // initialize the serial port:
  Serial.begin(38400);
  inputString.reserve(200);

  pinMode(homeSwitch, INPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  Tlc.init();
  setAll(0);
  //myStepper.setSpeed(100);
  //returnToHome();
  //Serial.println("F");
  //for(c=0; c < pre_load_count; c++) {
  //  Serial.println("L");
  //}

}

void setGreyLeds(char *s, int rows)
{  
  int r;
 
  Tlc.clear();
  for(r=0; r < rows; r++) {
    //Zero terminate the value
    s[4] ='\0';
    Tlc.set(r, atol(s));
    //Serial.println(atol(s));
    s +=5;
   
  }
  Tlc.update();
}

void setLeds(char *s) 
{
  int col; 
  int shiftc;
  
  col = atol(s);
  //Serial.println(col);
  Tlc.clear();
  for(shiftc = 0; shiftc < 16; shiftc++) {
    Tlc.set(shiftc, col & 0x01 ? 4095 : 0);
    col = col >> 1;
  }
  Tlc.update();
}

long col_size = 25;

void loop() {
  unsigned long col;
  int cmd;
  long col_delay = 1000;
  
  if(moving) {
    step(1, 1, col_delay);
    moving--;
    return;
  }

  if(stringComplete) {
    cmd = inputString.charAt(0);
    switch(cmd) {
      // Frame start command, switch off and return to home
    case 'F':
      setAll(0);
      Tlc.update();
      returnToHome();
      Serial.println("OK");
      break;
      
    case 'S':
    // Set column size
      col_size = atol(&inputString[1]);
      Serial.println("OK"); 
      break;
      
    case 'D':
    // Set forward speed
      col_delay = atol(&inputString[1]);
      Serial.println("OK");
      break;
      

    case 'C':
      // Column forward command
      setLeds(&inputString[1]);
      Serial.println("OK"); //Ack before step to double buffer
      step(1,col_size, col_delay);
//      setAll(0);
//      step(1,col_size, col_delay);
      break;
      
    case 'G':
      //Greyscaled column
      Serial.println("OK");
      setGreyLeds(&inputString[2], 16);
      //step(1,col_size, col_delay);
      move(col_size);
      break;
      
    case 'B':
      // Column backward command
      setLeds(&inputString[1]);
      Serial.println("OK"); //Ack before step to double buffer
     step(0,col_size, col_delay);
//      setAll(0);
//      step(0,col_size, col_delay);
      break;

      // Bad commands go here.
    default:
      setAll(4095);
      delay(1000);
      setAll(0);
      Serial.println("???");
      break;
    }

    inputString = "";
    stringComplete = false;
  } 
  else {
    delay(10);
  }
}




void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag˜
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}





