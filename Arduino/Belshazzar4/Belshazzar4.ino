
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

#include "SparkFun_Tlc5940.h"
#include <stdarg.h>

#define BACKWARDS HIGH
#define FORWARDS LOW
#define SLOW 4000
#define FAST 2000
#define BAUD_RATE 115200

#define NUM_LEDS 48
#define MAX_POS 6000
#define HOME_EASING_LIMIT 100

//const int stepsPerRevolution = 48*8;  // change this to fit the number of steps per revolution
// for your motor
const int homeSwitch = A0;
const int stepPin = 4;
const int dirPin = 7;
const int enablePin = 8;


int moving = 0;
long current_pos = 0;

void p(char *fmt, ... ) {
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

void alert(char *msg) {
  Serial.println(msg);
}

void returnToHome() {
  while (digitalRead(homeSwitch)) {
    setAll(0);
    //Tlc.set(current_pos % 48, 4095);
    Tlc.update();
    step(BACKWARDS, 1, FAST);
  }
  delay(500);
  // Move forward to allow for leeway on return journey
  step(FORWARDS, 50, SLOW);
  current_pos = 0;
}




void setAll(int val) {
  int l;
  //  Tlc.clear();
  //  Tlc.update();
  for (l = 0; l < NUM_LEDS; l++) {
    Tlc.set(l, val);
  }
  Tlc.update();
}

void step(int dir, int steps, int speed)
{
  int x;
  //Soft limit
  if (dir == FORWARDS && current_pos + steps > MAX_POS) {
    alert("Soft limit exceeded");
    return;
  }
  digitalWrite(dirPin, dir);
  for (x = 0; x < steps; x++) {
    // Hard limit
    if ((!digitalRead(homeSwitch)) && dir == BACKWARDS) {
      alert("Hard limit reached");
      return;
    }
    //digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
    //delayMicroseconds(000);
    digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
    digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
    //Go slowly when nearing the hard limit
    //Serial.println(current_pos);
    if ((dir == BACKWARDS && current_pos < HOME_EASING_LIMIT) ) {
      delayMicroseconds(SLOW);
    } else {
      delayMicroseconds(speed);
    }
    //delayMicroseconds(SLOW);
    if (dir == FORWARDS) {
      current_pos ++;
    } else {
      current_pos --;
    }
  }

}


static long moveStart;

void move(int steps)
{
  moveStart = micros();
  moving = steps;
}


int stepCount = 0;         // number of steps the motor has taken
String inputString = "";
int stringComplete = false;

void setup() {
  const int pre_load_count = 10;
  int c;

  // initialize the serial port:
  Serial.begin(BAUD_RATE);
  inputString.reserve(500);

  pinMode(homeSwitch, INPUT_PULLUP);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW);

  Tlc.init();
  setAll(10);
  //myStepper.setSpeed(100);
  //returnToHome();
  //Serial.println("F");
  //for(c=0; c < pre_load_count; c++) {
  //  Serial.println("L");
  //}
  moveStart = micros();

}

void setGreyLeds(char *s, int rows)
{
  int r;
  int brightness;

  Tlc.clear();
  for (r = 0; r < rows; r++) {
    //Zero terminate the value
    s[2] = '\0';
    //Tlc.set(r, atol(s));
    brightness = strtol(s, NULL, 16)*16;
    Tlc.set(r, brightness ? 4095 : 0);
    //Serial.println(atol(s));
    s += 3;

  }
  Tlc.update();
}

//void setLeds(char *s)
//{
//  int col;
//  int shiftc;
//
//  col = atol(s);
//  //Serial.println(col);
//  Tlc.clear();
//  for (shiftc = 0; shiftc < 16; shiftc++) {
//    Tlc.set(shiftc, col & 0x01 ? 4095 : 0);
//    col = col >> 1;
//  }
//  Tlc.update();
//}

long col_size = 20;
long col_delay = 1000;

void loop() {
  unsigned long col;
  int cmd;

  if (moving && micros() > moveStart + col_delay) {
    step(FORWARDS, 1, 1);
    moving--;
    moveStart = micros();
  }

  //return;


  if (stringComplete) {
    cmd = inputString.charAt(0);
    switch (cmd) {
      // Frame start command, switch off and return to home
      case 'F':
        setAll(0);
        Tlc.update();
        returnToHome();
        Serial.println("OK");
        setAll(0);
        Tlc.update();
        break;

      case 'S':
        // Set column size
        col_size = atol(&inputString[1]);
        Serial.print("COL SIZE");
        Serial.println(col_size);
        Serial.println("OK");
        break;

      case 'D':
        // Set forward speed
        col_delay = atol(&inputString[1]);
        Serial.println("OK");
        break;

      case 'M':
        // Move forward command
        Serial.println("Got M cmd");
        Serial.println("OK"); //Ack before step for continous movement
        move(atol(&inputString[1]));
        break;

      case 'C':
        // Column forward command
        //setLeds(&inputString[1]);
        Serial.println("OK"); //Ack before step to double buffer
        step(FORWARDS, col_size, SLOW);
        //      setAll(0);
        //      step(1,col_size, col_delay);
        break;

      case 'G':
        //Greyscaled column
        //
        setGreyLeds(&inputString[2], 48);
        //        Serial.println("Send ACK for G");
        Serial.println("OK");
        //        //step(FORWARDS, col_size, col_delay);
        //        //step(FORWARDS,col_size, FAST);
        move(col_size);
        //delayMicroseconds(FAST * col_size);
        break;

      case 'Z':
        // Range check command
        Serial.println("OK"); //Ack before step to double buffer
        step(FORWARDS, 500, SLOW);
        //      setAll(0);
        //      step(0,col_size, col_delay);
        break;

      case 'B':
        // Column backward command
        //setLeds(&inputString[1]);
        Serial.println("OK"); //Ack before step to double buffer
        step(BACKWARDS, col_size, col_delay);
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
    //delay(10);
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





