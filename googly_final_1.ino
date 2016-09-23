// # vim:expandtab:shiftwidth=4:tabstop=4:smarttab:autoindent:autoindent:syntax=cpp
// Halloween Eyes
// Brian Enigma, <brian@netninja.com>
// http://nja.me/eyes
//
// Based in part on the "Roboface" code from Adafruit:
// https://github.com/adafruit/Adafruit-LED-Backpack-Library/blob/master/examples/roboface/roboface.pde
//
// ----------------------------------------
//
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
//
// Written by P. Burgess for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <CapacitiveSensor.h>

#define MOTION_LED 13
#define LEFT_EYE  0
#define RIGHT_EYE 1
Adafruit_8x8matrix eyeMatrix[2];

// Rather than assigning matrix addresses sequentially in a loop, each
// has a spot in this array.  This makes it easier if you inadvertently
// install one or more matrices in the wrong physical position --
// re-order the addresses in this table and you can still refer to
// matrices by index above, no other code or wiring needs to change.
static const uint8_t matrixAddr[] = { 0x70, 0x71 };

static const uint8_t PROGMEM // Bitmaps are stored in program memory
blinkImg[][8] = {
  { B11111111,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B11111111 },
  { B00000000,
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B00000000,
    B00111100,
    B11111111,
    B11111111,
    B11111111,
    B00111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00111100,
    B11111111,
    B01111110,
    B00011000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B10000001,
    B01111110,
    B00000000,
    B00000000 } },
leftAngryImg[][8] = {
  { B10000000,
    B11110000,
    B11111110,
    B01111111,
    B01111111,
    B00111111,
    B00011110,
    B00000000 },
  { B00000000,
    B00000000,
    B11111110,
    B01111111,
    B01111111,
    B00111111,
    B00011110,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B01111111,
    B01111111,
    B00111111,
    B00011110,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B01111111,
    B00111111,
    B00011110,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00111111,
    B00000000,
    B00000000 } },
rightAngryImg[][8] = {
  { B00000001,
    B00001111,
    B01111111,
    B11111110,
    B11111110,
    B11111100,
    B01111000,
    B00000000 },
  { B00000000,
    B00000000,
    B01111111,
    B11111110,
    B11111110,
    B11111100,
    B01111000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B11111110,
    B11111110,
    B11111100,
    B01111000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111110,
    B11111100,
    B01111000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111100,
    B00000000,
    B00000000 } },
leftAngrierImg[][8] = {
  { B11000000,
    B11100000,
    B11111000,
    B01111100,
    B01111110,
    B01111111,
    B00111111,
    B00011110 },
  { B00000000,
    B00000000,
    B11111000,
    B01111100,
    B01111110,
    B01111111,
    B00111111,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B01111100,
    B01111110,
    B01111111,
    B00111111,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B01111111,
    B01111111,
    B00111111,
    B00000000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B01111111,
    B00000000,
    B00000000 } },
rightAngrierImg[][8] = {
  { B00000011,
    B00000111,
    B00011111,
    B00111110,
    B01111110,
    B11111110,
    B11111100,
    B01111000 },
  { B00000000,
    B00000000,
    B00011111,
    B11111110,
    B11111110,
    B11111100,
    B01111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B11111110,
    B11111110,
    B11111100,
    B01111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B11111110,
    B11111110,
    B11111100,
    B00000000,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111110,
    B00000000,
    B00000000,
    B00000000 } };

uint8_t
    blinkIndex[] = { 1, 2, 3, 4, 3, 2, 1 }, // Blink bitmap sequence
    blinkCountdown = 100, // Countdown to next blink (in frames)
    gazeCountdown  =  75, // Countdown to next eye movement
    gazeFrames     =  50, // Duration of eye movement (smaller = faster)
    gazeSubCountdown = 0;

int8_t
    eyeX = 3, eyeY = 3,   // Current eye position
    newX = 3, newY = 3,   // Next eye position
    dX   = 0, dY   = 0;   // Distance from prior to new position


int inputStep = 0;
int xCord = 0;
int yCord = 0;

//IR sensors
int n_sensor;
int
    sensor1 = 0, sensor2 = 0, sensor3 = 0,
    sensor4 = 0, sensor5 = 0, sensor6 = 0,
    sensor7 = 0, sensor8 = 0, sensor9 = 0,
    sensor10 = 0;
///////////////////////////   
    
const int buttonPin = 52; //On-off, mode change button
const int recPin = 50; //record button
const int ledPin = 13;

unsigned long previousMillis_led = 0;
boolean isPressing = false;
long interval_led = 5000;

int prerecbuttonState=0;
int buttonState = 0;
int lastButtonState = 0;
int buttonCount = 0;
int mode = 2;
boolean turningOn = false;
int recButtonState = 0;
unsigned long prevButtonMillis = 0;

//capacitive sensor
const int numReadings = 5;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;
CapacitiveSensor   cs_44_42 = CapacitiveSensor(44,42); 
bool sent = false;
//////////////////////////////////

int8_t eyeOffset = 0;
int disConn = 0;

enum state {
    OFF,
    ANGRY,
    IDLE
};

void initEye() {
    eyeMatrix[LEFT_EYE].setRotation(3);
    eyeMatrix[RIGHT_EYE].setRotation(3);
    eyeMatrix[LEFT_EYE].clear();
    eyeMatrix[RIGHT_EYE].clear();
}

void handleOff() {
    eyeMatrix[LEFT_EYE].clear();
    eyeMatrix[RIGHT_EYE].clear();
}

void handleAngry() {
    const uint8_t *leftImageOffset, *rightImageOffset;
    initEye();
    n_sensor = detectIR();
    if (n_sensor > 0) {
      leftImageOffset = &leftAngrierImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
      ][0] + eyeOffset;
    rightImageOffset = &rightAngrierImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
      ][0] + eyeOffset;
    eyeMatrix[LEFT_EYE ].drawBitmap(0, 0, leftImageOffset, 8, 8, LED_ON);
    eyeMatrix[RIGHT_EYE].drawBitmap(0, 0, rightImageOffset, 8, 8, LED_ON);
    eyeMatrix[LEFT_EYE].fillRect(4, 5, 2, 2, LED_OFF);
    eyeMatrix[RIGHT_EYE].fillRect(2, 5, 2, 2, LED_OFF);
    } else {
      leftImageOffset = &leftAngryImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
      ][0] + eyeOffset;
    rightImageOffset = &rightAngryImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
      ][0] + eyeOffset;
    eyeMatrix[LEFT_EYE ].drawBitmap(0, 0, leftImageOffset, 8, 8, LED_ON);
    eyeMatrix[RIGHT_EYE].drawBitmap(0, 0, rightImageOffset, 8, 8, LED_ON);
    eyeMatrix[LEFT_EYE].fillRect(4, 4, 2, 2, LED_OFF);
    eyeMatrix[RIGHT_EYE].fillRect(2, 4, 2, 2, LED_OFF);
    }


}

void handleIdle() {

}

int detectCam(int& x, int& y) {
  char inputChar;
  int step = 0;
  int _x, _y;
  while (Serial.available() > 0) {
          inputChar = Serial.read();
          if (step == 2) {
            if (inputChar == 'x') {
              x = _x;
              y = _y;
              return true;
            } else {
              step = 0;
            }
          } else if (step == 0) {
              _x = int(inputChar - '0');
              step = 1;
          } else if (step == 1) {
              _y = int(inputChar - '0');
              step = 2;
          }
  }
  return false;
}

int cnt = 0;

int detectIR() {
    sensor1 = digitalRead(3);
    sensor2 = digitalRead(4);
    sensor3 = digitalRead(5);
    sensor4 = digitalRead(6);
    sensor5 = digitalRead(7);
    sensor6 = digitalRead(8);
    sensor7 = digitalRead(9);
    sensor8 = digitalRead(10);
    sensor9 = digitalRead(11);
    sensor10 = digitalRead(12);

    return n_sensor = (1 - sensor1) + (1 - sensor2) + (1 - sensor3) +
    (1 - sensor4) + (1 - sensor5) + (1 - sensor6) + (1 - sensor7) +
    (1 - sensor8) + (1 - sensor9) + (1 - sensor10);
}

void setup() {
  
    pinMode(buttonPin, INPUT);
    pinMode(recPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    Serial.begin(9600);
    
    eyeMatrix[LEFT_EYE].begin(matrixAddr[LEFT_EYE]);
    eyeMatrix[RIGHT_EYE].begin(matrixAddr[RIGHT_EYE]);
    
    cs_44_42.set_CS_AutocaL_Millis(0xFFFFFFFF);  //Calibrating touch sensor
    
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}



void loop() {
    boolean received;
    unsigned long currentMillis_led = millis();
    
    prerecbuttonState = recButtonState;
    buttonState = digitalRead(buttonPin);
    recButtonState = digitalRead(recPin);
    
    //running avg of touch data
    //total = total - readings[readIndex];
    //readings[readIndex] = cs_44_42.capacitiveSensor(30);
    //total = total + readings[readIndex];
    //readIndex = readIndex + 1;
    //if (readIndex >= numReadings) {
    //readIndex = 0;
    //}
    total = 0;
    for (int i = 0; i < numReadings; i++) {
      total += cs_44_42.capacitiveSensor(30);
    }
    average = total / numReadings;
    
    ////////////////////////////////////////
    if (buttonState == HIGH) {
        if (buttonState != lastButtonState) {
            prevButtonMillis = currentMillis_led;
            turningOn = false;
        } else {
            if (currentMillis_led - prevButtonMillis > 3000) {
                prevButtonMillis = currentMillis_led;
                if (mode == 0) {
                    mode = 2;
                    turningOn = true;
                } else {
                    mode = 0;
                }
            }
        }
    } else {
        if (buttonState != lastButtonState) {
            if (mode == 1) {
                mode = 2;                         
            } else if (mode == 2 && !turningOn) {
                mode = 1;
            }
        }
    }
    lastButtonState = buttonState;
    
   // **** needs help
    if(recButtonState == HIGH && prerecbuttonState ==LOW){ 
    Serial.println(mode);
    }
 
    if (recButtonState == LOW && prerecbuttonState == HIGH && currentMillis_led - previousMillis_led <= interval_led){
      Serial.println(mode);
    }
    if (currentMillis_led - previousMillis_led >= interval_led ){
      if (!sent) {
        Serial.println(mode);
        sent = true;
      }
    }
    
// touch input

    if (average > 20){
     if (mode == 1) {
      Serial.println('3');
     } else if (mode == 2){
       Serial.println('4');
     }
    }      
////////////////////////  
   
    if (recButtonState == LOW) {
      isPressing = false;
      digitalWrite(ledPin, LOW);
      previousMillis_led = currentMillis_led;
      sent = false;
    } else {
      if (isPressing) {
        if (currentMillis_led - previousMillis_led > interval_led) {
          digitalWrite(ledPin, LOW);
        }else {
          digitalWrite(ledPin, HIGH);
        }
        
      } else {
        isPressing = true;
        digitalWrite(ledPin,HIGH);
      }
    }
            

   
    const uint8_t *imageOffset;

    switch (mode) {
        // mode - 0: off, 1: angry, 2: idle
        case 0:
            handleOff();
            break;
        case 1:
            handleAngry();
            break;
        case 2:
            received = detectCam(xCord, yCord);
            
            if (!received) {
                n_sensor = detectIR();
            }
            
            initEye();

   

    imageOffset = &blinkImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
      ][0] + eyeOffset;
    eyeMatrix[LEFT_EYE ].drawBitmap(0, 0, imageOffset, 8, 8, LED_ON);
    eyeMatrix[RIGHT_EYE].drawBitmap(0, 0, imageOffset, 8, 8, LED_ON);

    if(--gazeCountdown <= gazeFrames) {
            // Eyes are in motion - draw pupil at interim position
            eyeMatrix[LEFT_EYE].fillRect(
              newX - (dX * gazeCountdown / gazeFrames),
              newY - (dY * gazeCountdown / gazeFrames),
              2, 2, LED_OFF);
            eyeMatrix[RIGHT_EYE].fillRect(
              newX - (dX * gazeCountdown / gazeFrames),
              newY - (dY * gazeCountdown / gazeFrames),
              2, 2, LED_OFF);
        if(gazeCountdown == 0) {    // Last frame?
            eyeX = newX; eyeY = newY; // Yes.  What's new is old, then...
            if (n_sensor > 0){
                newX = 3 + ((-2*(1-sensor8)) + (-3*(1-sensor9)) + (-2*(1-sensor10)) + (-1*(1-sensor1)) + (1*(1-sensor2)) + (2*(1-sensor3)) + (3* (1-sensor4)) + (2* (1-sensor5)) + (1* (1-sensor6)) + (-1* (1-sensor7)) ) / n_sensor ;
                newY = 3 + ((2*(1-sensor8)) + (1*(1-sensor9)) + (-2*(1-sensor10)) + (-3*(1-sensor1)) + (-3*(1-sensor2)) + (-2*(1-sensor3)) + (1* (1-sensor4)) + (2* (1-sensor5)) + (3* (1-sensor6)) + (3* (1-sensor7)) ) / n_sensor ;
                gazeFrames    = 3;
                gazeCountdown = 3;

              } else if (received){
                newX = xCord;
                newY = yCord;
                gazeFrames    = 3;
                gazeCountdown = 3;
              } else {
                if (cnt > 28) {
                  cnt = 0;
                  newX = 3; newY = 3;
                  gazeFrames    = random(8, 10);      // Duration of eye movement
                gazeCountdown = random(gazeFrames, 7); // Count to end of next movement
                } else {
                  cnt++;
                  gazeFrames    = 3;
                gazeCountdown = 3;
                }               
              }
              dX = newX - eyeX;             // Horizontal distance to move
              dY = newY - eyeY;             // Vertical distance to move
            }
          } else {
            // Not in motion yet -- draw pupil at current static position
            eyeMatrix[LEFT_EYE ].fillRect(eyeX, eyeY, 2, 2, LED_OFF);
            eyeMatrix[RIGHT_EYE].fillRect(eyeX, eyeY, 2, 2, LED_OFF);
          }
          
          break;
        }

        if(--blinkCountdown == 0) blinkCountdown = random(5, 180);
        // Refresh all of the matrices in one quick pass
        eyeMatrix[LEFT_EYE ].writeDisplay();
        eyeMatrix[RIGHT_EYE].writeDisplay();
        delay(30); // ~50 FPS
        
       

    /*
     * TODO:
     * 1. blink between mode switch
     * 2. Off - close eye
     * 3. Recording
     */

}
