#include <Wire.h>
#include "Keyboard.h"
#include <Encoder.h> //Library for simple interfacing with encoders (up to two)

Encoder RotaryEncoderA(10, 16); //the LEFT encoder (encoder A)
Encoder RotaryEncoderB(14, 15);  //the RIGHT encoder (encoder B)


#include <Keypad.h>

const byte ROWS = 2; //two rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1', '2', '3', '4'},  //  the keyboard hardware is a 2x4 grid... 
  {'5', '6', '7', '8'},
};

// Variables that will change:

long positionEncoderA  = -999; //encoderA LEFT position variable
long positionEncoderB  = 0; //encoderB RIGHT position variable

//const int ModeButton = A0;    // the pin that the Modebutton is attached to
const int pot = A1;           // pot for adjusting attract mode demoTime or mouseMouse pixel value
const int Mode1= A2;
const int Mode2= A3;          //Mode status LEDs

byte rowPins[ROWS] = {4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9 }; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
  pinMode(Mode1,OUTPUT); digitalWrite(Mode1,LOW);
  pinMode(Mode2,OUTPUT); digitalWrite(Mode2,LOW);

  Keyboard.begin();
}


void loop() 
{
  char key = keypad.getKey();
  
  encoderA();
  encoderB();
  
  if (key) 
  {
    switch (key) 
    {
//        case '1': Keyboard.println("check alarm"); break;
//        case '2': Keyboard.println("set"); break;
//        case '3': Keyboard.println("arm alarm"); break;
//        case '4': Keyboard.println("disarm alarm"); break;
        case '5': Keyboard.println("backlight"); break;
//        case '6': Keyboard.println("pause Sonos"); break;
        case '7': Keyboard.println("arm alarm"); break;
        case '8': Keyboard.println("set"); break;
       
//        delay(50); Keyboard.releaseAll(); // this releases the buttons 
    }  
  }
}

void encoderA(){ 
  long newPos = RotaryEncoderA.read()/4;
  
  if (newPos != positionEncoderA && newPos > positionEncoderA) {
    positionEncoderA = newPos;
     Keyboard.println("q"); // left up
     }

  if (newPos != positionEncoderA && newPos < positionEncoderA) {
    positionEncoderA = newPos;
    Keyboard.println("a"); //left down

  }
}

void encoderB(){
//  long temp = RotaryEncoderB.read()/4; //When the encoder lands on a valley, most encoders use an increment of 4.

//  if (temp < 0)
//  { RotaryEncoderB.write(236); }
//  else if (temp > 59)
//  { RotaryEncoderB.write(0); }
  
  long newPos = RotaryEncoderB.read()/4;

//  Keyboard.println("NewPos: " + newPos);

  
  if (newPos != positionEncoderB && newPos > positionEncoderB) {
    positionEncoderB = newPos;
     //Keyboard.println("B" + positionEncoderB);
     Keyboard.println("l"); //right down
     }

  if (newPos != positionEncoderB && newPos < positionEncoderB) {
    positionEncoderB = newPos;
    //Keyboard.println("B1" + positionEncoderB);
    Keyboard.println("p"); // right up

  }
}
