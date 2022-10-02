/*******************************************************************
 * A multi-mode Macro keyboard with Arduino Pro Micro using row column matrix-ing.
 * This is example sketch to understand key layout, assignments, and command protocols. 
 * This example is slightly more robust than the larger size MacroKeyboard (https://www.youtube.com/watch?v=acJ6gufBN_A.
 * Each mode (profile/application) has a unique definition for each encoder, including mouse control and a display LCD
 * to descibes the modes or key assignements. 
 * Some mouse movement get their value from analogRead(pin 0) so make sure something is connected.
 * Don't forget to solder all the diode and encoder solder jumpers on the backside!!
  (c) 2020 Ryan Bates 
  Last time I touched this: Oct 23 2020
  web: www.retrobuiltgames.com
  youtube: https://www.youtube.com/c/ryanbatesrbg
  twitter: @retrobuiltgames
  
Ryan's Pro Tips:
=============== Keyboard Control================================================================================
Keyboard.write();   Sends a keystroke to a connected computer. 
                    This is similar to pressing and releasing a key on your keyboard.
                    Will send a shift command if applicable. Example: Keyboard.write('K') will 
                    automatically do SHIFT + k. 
                    Can also accept ASCII code like this:
                    //Keyboard.write(32); // This is space bar (in decimal)
                    Helpful list of ASCII + decimal keys http://www.asciitable.com/
                    

Keyboard.press();   Best for holding down a key with multi-key commands; like copy/ paste
                    This example is [ctrl] + [shift] + [e] 
                      //Keyboard.press(KEY_LEFT_CTRL);
                      //Keyboard.press(KEY_LEFT_SHIFT);
                      //Keyboard.press('e'); 
                      //delay(100); Keyboard.releaseAll();
                

Keyboard.print();   Sends a keystroke(s)
                    Keyboard.print("stop using aim bot"); // types this in as a char or int! (not a string)!


Keyboard.println(); Sends a keystroke followed by a newline (carriage return)
                     Very practical if you want to type a password and login in one button press!
                     
SOMETIMES, applications are coded to recognize Keyboard.press() and not Keyboard.write() and vice versa.
You might have to experiment. 

=============== Mouse Control================================================================================
Mouse.move(x, y, wheel);  Moves the mouse and or scroll wheel up/down left/right.
                          Range is -128 to +127. units are pixels 
                          -number = left or down
                          +number = right or up

Mouse.press(b);         Presses the mouse button (still need to call release). Replace "b" with:
                        MOUSE_LEFT   //Left Mouse button
                        MOUSE_RIGHT  //Right Mouse button
                        MOUSE_MIDDLE //Middle mouse button
                        MOUSE_ALL    //All three mouse buttons
                        
Mouse.release(b);       Releases the mouse button.

Mouse.click(b);         A quick press and release.

*******************************************************************/
// ----------------------------
// Standard Libraries
// ----------------------------
#include <Wire.h>
#include "Keyboard.h"
#include <Mouse.h> //there are some mouse move functions for encoder_Mode 2 and 3



#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address for a 40 chars and 4 line display

const int LCD_NB_ROWS = 4 ;        //for the 4x20 LCD lcd.begin(), but i think this is kinda redundant 
const int LCD_NB_COLUMNS = 20 ;
unsigned long previousMillis = 0;     // values to compare last time interval was checked (For LCD refreshing)
int check_State = 0;                    // state to check trigger the demo interrupt

// Library with a lot of the HID definitions and methods
// Can be useful to take a look at it see whats available
// https://github.com/arduino-libraries/Keyboard/blob/master/src/Keyboard.h

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------
#include <Encoder.h> //Library for simple interfacing with encoders (up to two)
//low performance ender response, pins do not have interrupts
Encoder RotaryEncoderA(10, 16); //the LEFT encoder (encoder A)
Encoder RotaryEncoderB(14, 15);  //the RIGHT encoder (encoder B)

#include <Keypad.h>
// This library is for interfacing with the 4x4 Matrix
// 
// Can be installed from the library manager, search for "keypad"
// and install the one by Mark Stanley and Alexander Brevig
// https://playground.arduino.cc/Code/Keypad/

//Define the rols column matrix roots
const byte ROWS = 2; //two rows
const byte COLS = 4; //four columns


//define the physical matrix layout
char keys[ROWS][COLS] = {
  {'1', '2', '3', '4'},  //  the keyboard hardware is a 2x4 grid... 
  {'5', '6', '7', '8'},

};
// The library will return the character inside this array when the appropriate
// button is pressed then look for that case statement. This is the key assignment lookup table.
// Layout(key/button order) looks like this
//     |------------------------|
//     |   [6/5]*               |     *TRS breakout connection. Keys 5 and 6 are duplicated at the TRS jack
//     |      [1] [2] [3] [4]   |     * Encoder A location = key[1]      
//     |      [5] [6] [7] [8]   |     * Encoder B location = Key[4]
//     |------------------------|



// Variables that will change:
int modePushCounter = 0;     // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int mouseMove;
String password;             // string for rando password generator, its a global variable because i might do something fancy with it?

long positionEncoderA  = -999; //encoderA LEFT position variable
long positionEncoderB  = -999; //encoderB RIGHT position variable

const int ModeButton = A0;    // the pin that the Modebutton is attached to
const int pot = A1;           // pot for adjusting attract mode demoTime or mouseMouse pixel value
const int Mode1= A2;
const int Mode2= A3;          //Mode status LEDs

byte rowPins[ROWS] = {4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8, 9 }; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
  lcd.init();       //initialize the 4x20 lcd
  lcd.backlight();  //turn on the backlight
  lcd.begin(LCD_NB_COLUMNS , LCD_NB_ROWS);
  randomSeed(analogRead(0));           // is this a bad idea? it's called before assigning the button as pull up... the switch is open at rest..
  pinMode(ModeButton, INPUT_PULLUP);  // initialize the button pin as a input:  
  pinMode(Mode1,OUTPUT); digitalWrite(Mode1,LOW);
  pinMode(Mode2,OUTPUT); digitalWrite(Mode2,LOW);

  lcd.setCursor(0, 0);  lcd.print("Mini Macro KB Demo");
  lcd.setCursor(0, 1);  lcd.print("(c) 2020 Ryan Bates");
  delay(800);
  lcd.clear();

  Keyboard.begin();
}


void loop() {
char key = keypad.getKey();
mouseMove = (analogRead(pot)); //reading the analog input, pot = pin A1
mouseMove = map(mouseMove, 0,1023, 1,124); //remap the analog pot values fron 1 to 124
checkModeButton();

  switch (modePushCounter) { // switch between keyboard configurations:
    case 0:    //  Application Alpha or MODE 0 ========================================================================
      encoderA_Mode0();
      encoderB_Mode0();
      LCD_update_0();
      digitalWrite(Mode1,LOW); digitalWrite(Mode2,LOW);   //indicate what mode is loaded
       if (key) {
    //Serial.println(key);
    switch (key) {
      case '1': Keyboard.println("Alpha key1"); break;    //don't forget the break statement!
      case '2': RandoPasswordGenerator(); break;           //a not-so-sophisticated password generator
      
      case '3': Keyboard.press(KEY_LEFT_CTRL);   
                Keyboard.print('z');                       //undo
                break;
      case '4': lcd.setCursor(0,3); lcd.print("                    "); //overwrite the bottom LCD row
                lcd.setCursor(0,3); 
                break; 
      case '5': Keyboard.press(KEY_LEFT_CTRL);   
                Keyboard.print('a');                        //select all    
                break;
      case '6': Keyboard.press(KEY_LEFT_CTRL); 
                Keyboard.print('c');                         //copy
                break;
                
      case '7': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('v');                         // paste
                break;
                
      case '8': Keyboard.press(KEY_DELETE); 
                 break; }
    delay(50); Keyboard.releaseAll(); // this releases the buttons 
                }
      break;
      
    case 1:    //  Arduino shortcuts or MODE 1 ========================================================================
      encoderA_Mode1();
      encoderB_Mode1();
      LCD_update_1();
      digitalWrite(Mode1,HIGH); digitalWrite(Mode2,LOW);
      if (key) {
    //Serial.println(key);
    switch (key) {
      case '1': Keyboard.write(32);                          //Space Bar
                break;    
      case '2': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('r');                          //Verify
                break;    
      case '3': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('u');                          //Upload
                break;    
      case '4': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.press(KEY_LEFT_SHIFT);
                Keyboard.print('m');                          //Serial Monitor
                break;    
      case '5': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('c');                          //Copy
                break;
      case '6': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('v');                          //Paste
                break;
      case '7': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('/');                          //Comment / Uncomment
                break;
      case '8': Keyboard.press(KEY_LEFT_CTRL);  
                Keyboard.print('f');                          //Find
                break; }
    delay(50); Keyboard.releaseAll(); // this releases the buttons 
               }
      break;
      
    case 2:    // Application Delta or MODE 2 ========================================================================
               //this mode mimics a password manager. Types ['login ID'] + [tab] + ['password'] + [enter]
               //This is not a really secure way manage your passwords, but it's an idea...
      encoderA_Mode2();
      encoderB_Mode2();
     
      digitalWrite(Mode1,LOW); digitalWrite(Mode2,HIGH);
    if (key) {
    switch (key) {
      case '1': Keyboard.println("Delta key1"); break;
      case '2': Keyboard.println("Delta key2"); break;
      case '3': Keyboard.println("Delta key3"); break;
      case '4': Keyboard.println("Delta key4"); break;
      
      case '5': Keyboard.print("admin_1");      Keyboard.write(9); //tab key
                Keyboard.println("password_1"); break;
      case '6': Keyboard.print("admin_2");      Keyboard.write(9); //tab key
                Keyboard.println("password_2"); break;
      case '7': Keyboard.print("admin_3");      Keyboard.write(9); //tab key
                Keyboard.println("password_3"); break;
      case '8': Keyboard.print("admin_4");      Keyboard.write(9); //tab key
                Keyboard.println("password_4"); break;
      
 }
    delay(50); Keyboard.releaseAll(); // this releases the buttons 
              }
      break;
      
    case 3:    // Application Delta or MODE 3 ========================================================================
     digitalWrite(Mode1,HIGH); digitalWrite(Mode2,HIGH);
      encoderA_Mode3(); //etch-a-sketch with the mouse up/down
      encoderB_Mode3(); //etch-a-sketch with the mouse left/right
      LCD_update_3();
   if (key) {
    //Serial.println(key);
    switch (key) {
      case '1': Keyboard.println("Hi.   Hopefully this example code makes sense."); break;
      
      case '2': //macro that opens chrome and a random wiki page for learning.
        Keyboard.press(KEY_LEFT_GUI); 
        Keyboard.press('r'); 
        Keyboard.release(KEY_LEFT_GUI); 
        Keyboard.release('r'); 
        delay(50); //give your system time to catch up with these android-speed keyboard presses
        Keyboard.println("chrome"); delay(500);
        Keyboard.println("https://en.wikipedia.org/wiki/Special:Random"); break; 

        
      case '3': Keyboard.press(KEY_LEFT_GUI); delay(50);
                Keyboard.release(KEY_LEFT_GUI);delay(400);
                Keyboard.println("mag"); delay(200);
           
      break;
      
      case '4': Keyboard.write(32); break; //smashing that space bar
      case '5': //macro that opens Chrome & Rick Rolls you like a chump
        Keyboard.press(KEY_LEFT_GUI); 
        Keyboard.press('r'); 
        Keyboard.release(KEY_LEFT_GUI); 
        Keyboard.release('r'); 
        delay(50); //give your system time to catch up with these android-speed keyboard presses
        Keyboard.println("chrome"); delay(500);
        Keyboard.println("https://www.youtube.com/watch?v=dQw4w9WgXcQ"); 
        break;

      
      case '6':  //macro that opens Chrome and goes to my youtube channel!
        Keyboard.press(KEY_LEFT_GUI); 
        Keyboard.press('r'); 
        Keyboard.release(KEY_LEFT_GUI); 
        Keyboard.release('r'); 
        delay(50); //give your system time to catch up with these android-speed keyboard presses
        Keyboard.println("chrome"); delay(500);
        Keyboard.println("https://www.youtube.com/c/ryanbatesrbg"); 
        break;
      
      case '7': //macro example!!! Windows_Key+R = Run then type "mspaint" and press enter. Opens MS Paint
        Keyboard.press(KEY_LEFT_GUI); 
        Keyboard.press('r'); 
        Keyboard.release(KEY_LEFT_GUI); 
        Keyboard.release('r'); 
        delay(50); //give your system time to catch up with these android-speed keyboard presses
        Keyboard.println("mspaint"); 
        break;
      
      
      case '8': //macro example!!! Windows_Key+R = Run then type "calc" and press enter. Opens MS Calculator
        Keyboard.press(KEY_LEFT_GUI); 
        Keyboard.press('r'); 
        Keyboard.release(KEY_LEFT_GUI); 
        Keyboard.release('r'); 
        delay(50); //give your system time to catch up with these android-speed keyboard presses
        Keyboard.println("calc"); 
        break;
                  }
  
  delay(50); Keyboard.releaseAll(); // this releases the buttons 

}}
  }
void RandoPasswordGenerator(){
      long random_alpha;
      long random_ALPHA;
      long random_numeric;
      long random_special;
      
      char alpha;         // lower case letters
      char ALPHA;         // uppercase letters
      char number;        // numbers!
      char specialChar;   // !@#$%^&* and so on
      char randoPassword; // array to combine these all together
       
      char alphabet[]="qwertyuiopasdfghjklzxcvbnm"; // arrary for lower case letters. alphabet[0]; returns "q"...... alphabet[25]; returns "m"  alphabet[26]; will break something by accessing the null character
      char ALPHABET[]="QWERTYUIOPASDFGHJKLZXCVBNM"; //when declaring an array of type char, one more element than your initialization is required, to hold the required null character
      char numeric[] ="1234567890";
      char special[] ="!@#$%^&*()";
      
      random_alpha = random(sizeof(alphabet));    //random(max) sets the limit of the random number being pulled
      random_ALPHA = random(sizeof(ALPHABET));    //sets the random number upper limit to the size of char array
      random_numeric = random(sizeof(numeric));
      random_special = random(sizeof(special));   
      
      alpha = alphabet[random_alpha]; //picks a random character in the respective array. 
      ALPHA = ALPHABET[random_ALPHA];
      number = numeric[random_numeric];
      specialChar = special[random_special];
      
      String alpha_String = String(alpha); //convert all the single chars to strings...so we can concatenate them...
      String ALPHA_String = String(ALPHA);
      String number_String = String(number);
      String specialChar_String = String(specialChar);
      
      password = (alpha_String + ALPHA_String + number_String + specialChar_String); //concatenate the random alpha and numerics to format: xX$!
      //this is great and all, but it will always generate a pattern, which isnt secure with machine learning (or any basic pattern recognition)
           
      password.toCharArray(randoPassword,sizeof(randoPassword)); //convert the xX# string to char datatype
      Keyboard.print(password);  
      lcd.print(password);
      }
  
void checkModeButton(){
  buttonState = digitalRead(ModeButton);
  if (buttonState != lastButtonState) { // compare the buttonState to its previous state
    if (buttonState == LOW) { // if the state has changed, increment the counter
      // if the current state is LOW then the button cycled:
      modePushCounter++;
    } 
    delay(50); // Delay a little bit to avoid bouncing
  }
  lastButtonState = buttonState; // save the current state as the last state, for next time through the loop
   if (modePushCounter >3){ //reset counter after 4 presses (remember we start counting at 0) change this number to add more modes
      modePushCounter = 0;}
}
//=============== encoder definitions/assignments ===========================================
//this section allows a unique encoder function for each mode (profile). Four total in this case or modes 0 through 3.

//=============Encoder A & B Function ====== Set 0 =========================================================
void encoderA_Mode0(){
  long newPos = RotaryEncoderA.read()/4; //When the encoder lands on a valley, this is an increment of 4.
                                          // your encoder might be different (divide by 2) i dunno. 
  if (newPos != positionEncoderA && newPos > positionEncoderA) {
    positionEncoderA = newPos;
    Keyboard.press(KEY_RIGHT_ARROW);
    Keyboard.release(KEY_RIGHT_ARROW);                      }

  if (newPos != positionEncoderA && newPos < positionEncoderA) {
    positionEncoderA = newPos;
    Keyboard.press(KEY_LEFT_ARROW);
    Keyboard.release(KEY_LEFT_ARROW);                      }
}

void encoderB_Mode0(){
  long newPos = RotaryEncoderB.read()/4; //When the encoder lands on a valley, most encoders use an increment of 4.
  if (newPos != positionEncoderB && newPos > positionEncoderB) {
    positionEncoderB = newPos;
     Keyboard.press(KEY_DOWN_ARROW);
    Keyboard.release(KEY_DOWN_ARROW);                      }

  if (newPos != positionEncoderB && newPos < positionEncoderB) {
    positionEncoderB = newPos;

    Keyboard.press(KEY_UP_ARROW);
    Keyboard.release(KEY_UP_ARROW);                         }
}
//=============Encoder A & B Function ====== Set 1 =========================================================
void encoderA_Mode1(){
  long newPos = RotaryEncoderA.read()/2; 
  if (newPos != positionEncoderA && newPos < positionEncoderA) {
    positionEncoderA = newPos;
     //tab increase
    Keyboard.write(9); //tab key
     }

  if (newPos != positionEncoderA && newPos > positionEncoderA) {
    positionEncoderA = newPos;
    //tab decrease
    Keyboard.press(KEY_LEFT_SHIFT); 
    Keyboard.write(9); //tab key
    Keyboard.release(KEY_LEFT_SHIFT);                      }

}

void encoderB_Mode1(){
  long newPos = RotaryEncoderB.read()/2; 
  if (newPos != positionEncoderB && newPos < positionEncoderB) {
    positionEncoderB = newPos;
      //Font decrease | Arduino IDE
    Keyboard.press(KEY_LEFT_CTRL); 
    Keyboard.press('-');
    Keyboard.release('-');  Keyboard.release(KEY_LEFT_CTRL);         }

  if (newPos != positionEncoderB && newPos > positionEncoderB) {
    positionEncoderB = newPos;
      //Font increase | Arduino IDE
    Keyboard.press(KEY_LEFT_CTRL); 
    Keyboard.press('=');
    Keyboard.release('=');   Keyboard.release(KEY_LEFT_CTRL);       }
}

//=============Encoder A & B Function ====== Set 2 =========================================================
void encoderA_Mode2(){ //testing some encoder wheel pay control for arcade games; centede, tempest...

  long newPos = RotaryEncoderA.read()/2; 
  if (newPos != positionEncoderA && newPos > positionEncoderA) {
    positionEncoderA = newPos;
    //Serial.println(mouseMove);
     Mouse.move(-mouseMove,0,0); //moves mouse right... Mouse.move(x, y, wheel) range is -128 to +127
                        }

  if (newPos != positionEncoderA && newPos < positionEncoderA) {
    positionEncoderA = newPos;
    Mouse.move(mouseMove,0,0); //moves mouse left... Mouse.move(x, y, wheel) range is -128 to +127
                         }
}

void encoderB_Mode2(){
  long newPos = RotaryEncoderB.read()/2; //When the encoder lands on a valley, this is an increment of 2.
  if (newPos != positionEncoderB && newPos < positionEncoderB) {
    positionEncoderB = newPos;
    Mouse.move(0,-mouseMove,0);                                                           }

  if (newPos != positionEncoderB && newPos > positionEncoderB) {
    positionEncoderB = newPos;
    Mouse.move(0,mouseMove,0);            
                                                              }
}

//=============Encoder A & B Function ====== Set 3 =========================================================
void encoderA_Mode3(){
  long newPos = RotaryEncoderA.read()/2; 
  if (newPos != positionEncoderA && newPos > positionEncoderA) {
    positionEncoderA = newPos;
    Mouse.press(MOUSE_LEFT); //holds down the mouse left click
    Mouse.move(-4,0,0); //moves mouse right... Mouse.move(x, y, wheel) range is -128 to +127
    Mouse.release(MOUSE_LEFT); //releases mouse left click
                                                               }

  if (newPos != positionEncoderA && newPos < positionEncoderA) { 
    positionEncoderA = newPos;
    Mouse.press(MOUSE_LEFT); //holds down the mouse left click
    Mouse.move(4,0,0); //moves mouse left... Mouse.move(x, y, wheel) range is -128 to +127
    Mouse.release(MOUSE_LEFT); //releases mouse left click                   
                                                              }
}

void encoderB_Mode3(){
  long newPos = RotaryEncoderB.read()/2;
  if (newPos != positionEncoderB && newPos > positionEncoderB) {
    positionEncoderB = newPos;
    Mouse.press(MOUSE_LEFT); //holds down the mouse left click
    Mouse.move(0,4,0); //moves mouse up... Mouse.move(x, y, wheel) range is -128 to +127  
    Mouse.release(MOUSE_LEFT); //releases mouse left click
                                                               }

  if (newPos != positionEncoderB && newPos < positionEncoderB) {
    positionEncoderB = newPos;
    Mouse.press(MOUSE_LEFT); //holds down the mouse left click
    Mouse.move(0,-4,0); //moves mouse down... Mouse.move(x, y, wheel) range is -128 to +127
    Mouse.release(MOUSE_LEFT); //releases mouse left click                   
                                                              }
}
