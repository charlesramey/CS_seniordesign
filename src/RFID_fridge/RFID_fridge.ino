#include "Wiegand.h"
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <string.h>
#include <Servo.h>

/* SD Card Hook Up Guide
    MOSI -> Pin 11
    MISO -> Pin 12
    CLK -> Pin 13
    CS -> 8 -> 14
*/

//PINS USED:
//RFID: 2, 3 LCD: 4, 5, 6, 7, 8, 9, 13 SD CARD: 14 SERVOS: 46 BUTTONS: 

//Define servos
Servo servo1;
Servo servo2;
Servo servo3;

//Define Servo Pins
const int servo1_pin = 46;
const int servo2_pin = 0;
const int servo3_pin = 0;//#######################################TODO GET PINS FOR SECOND AND THIRD SERVOS

//Requisite SD card variables
Sd2Card card;
SdVolume volume;
SdFile root;

//Users file from SD Card
File users_file;
int lastCardID = -1;

//Define which pin is used for SD card chip select
const int chip_select = 14;

//Define where the RFID reader is plugged in
const int clock_pin = 3;
const int data_pin = 2;

//Define where the Buttons are plugged in
const int button1_pin = 0;
const int button2_pin = 0;
const int button3_pin = 0;//#####################################TODO: INSERT PINS FOR ALL THREE BUTTONS

//Changing Button States
int button1State = 0;
int button2State = 0;
int button3State = 0;

//Card ID
long Card_ID; //type long for long int
int userCredits; // number of credits active user has
int dispense = 1; // value of one soda
int nextPointer; // if dispense, location of pointer

//Create RFID reader object
Wiegand reader;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons // Mark Bramwell, July 2010
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
  adc_key_in = analogRead(0);      // read the value from the sensor
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;
  return btnNONE;  // when all others fail, return this...
}

void setup()
{
  //servo setup
  servo1.attach(servo1_pin); //PIN 46 for first servo
  int pos1 = 100; //degree lim of dispenser
  servo1.write(4);
  servo2.attach(servo2_pin);
  servo2.write(4);
  servo3.attach(servo3_pin);
  servo3.write(4);
  
  Serial.begin(9600);
  reader.begin();
  // Attach the 0-bit Wiegand signal to Arduino's Interrupt 0 (Pin 2 for UNO)
  // Attach the 1-bit Wiegand signal to Arduino's Interrupt 1 (Pin 3 for UNO)
  reader.attach(0, 1);
  
  //Inorder to make the SD card library work, pin 10 must be set as an
  //output, even though we are using pin 8 as the chip select (CS)
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH); // For the backlight to stay on!
  
  //Set the chip_select pin to output fo SD reader
  pinMode(chip_select, OUTPUT);
  pinMode(button1_pin, INPUT);
  pinMode(button2_pin, INPUT);
  pinMode(button3_pin, INPUT);

  //Try to initialize the SD Card
  if (!SD.begin(chip_select))
  {
    Serial.println("SD Card Initialization Failed");
    return;
  }
  Serial.println("SD Card Succesfully Initialized");

  // now lines will be parsed and placed into arrays

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("||SodaBot v1.0||");
  lcd.setCursor(0, 1);
  lcd.print("Tap ur BuzzCard");
}


boolean readback = true; // if to read the csv again and update local array
boolean found = false; // if card ID is found
boolean scanned = false; // if a card has been scanned just then
boolean active = false; // if user has scanned card and may do something
boolean onetime = true;
boolean anothertime = true;

void loop()
{

  if (reader.available() == true)
  {
    lcd.setCursor(0, 0);
    lcd.print("                "); // clear lcd
    lcd.setCursor(0, 1);
    lcd.print("                ");
    active = true; // entrance to second loop
    scanned = true;
    dispense = 1;
    Card_ID = reader.getCardCode();
    Serial.print(" Facility Code = ");
    Serial.print(reader.getFacilityCode(), DEC);
    Serial.print(", Card Code = ");
    Serial.println(Card_ID, DEC);

    lcd.setCursor(0, 1);
    int IDpointer = findID(Card_ID);
    if (IDpointer > 0 ) {
      printCredits(IDpointer);
    }
    else {
      Serial.println("NO USER");
      active = false;
    }
    //    Serial.println(nextPointer);
    //    lcd.print(getCredits(Card_ID));

    reader.reset();
  }




  while (active) {
    detachInterrupt(2); // DISABLE SCANNER WHILE ACTIVE.
    detachInterrupt(3); //
    //lcd_key = read_LCD_buttons(); // read buttons
    lcd.setCursor(0, 1);

//BUTTON DEBAUCHERY
    if(button1State == HIGH){
      //dispense from dispenser #1
      servo1.attach(servo1_pin);
      //IF USER HAS 0 CREDITS
      if(userCredits == 0){
        lcd.print("NO CREDITS");
        dispense--;
        active = false;
        break;
      }
      
      while(dispense > 0){
        dispenseCan(servo1);
        Serial.println("DISPENSE 1");
        dispense--;
        active = false;
        break;
      }
    }
    if(button2State == HIGH){
      //dispense from dispenser #2
      servo2.attach(servo2_pin);
      
      //IF USER HAS 0 CREDITS
      if(userCredits == 0){
        lcd.print("NO CREDITS");
        dispense--;
        active = false;
        break;
      }
      
      while(dispense > 0){
        dispenseCan(servo2);
        Serial.println("DISPENSE 2");
        dispense--;
        active = false;
        break;
      }
    }
    if(button3State == HIGH){
      //dispense from dispenser #3
      servo3.attach(servo3_pin);
      
      //IF USER HAS 0 CREDITS
      if(userCredits == 0){
        lcd.print("NO CREDITS");
        dispense--;
        active = false;
        break;
      }
      
      while(dispense > 0){
        dispenseCan(servo3);
        Serial.println("DISPENSE 3");
        dispense--;
        active = false;
        break;
      }
    }
//          //      lcd.print("DOWN  ");
//          while (dispense > 0) { //because button press results in many calls
//            if (userCredits == 0) {
//              Serial.println("NO CREDITS");
//              lcd.print("NO CREDITS");
//              dispense--;
//              active = false;
//              break;
//            }
//            Serial.println("DOWN");
//            lowerCredits(nextPointer); //dispenseCan() is used in lowerCredits()
//            dispense--;
//            active = false;
////            dispenseCan(servo1);
//          }
//          break;
//        }
//      case btnSELECT:
//        {
//          lcd.print("SELECT");
//          break;
//        }
//      case btnNONE:
//        {
//          //      lcd.print("NONE  ");
//          break;
//        }
//    }
    reader.begin();       // RE-ENABLE SCANNER
    reader.attach(0, 1);
  }
}

int findID (long Card_ID) {
  String credits;
  users_file = SD.open("Users.csv"); // OPEN FILE
  if (!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  int n = 0; // to count number of consecutive character matches
  //        char data[users_file.size()];
  char c;
  while (users_file.available()) //file read
  {
    c = users_file.read();
    if (n == 5) {
      int number = users_file.position() + 1;
      users_file.close();
      return number;
    }
    if ( c == String((Card_ID))[n]) {
      if ( users_file.peek() == String((Card_ID))[n + 1]) {
        n++;
      }
      else {
        n = 0;
      }
    }
  }
  users_file.close(); // CLOSE FILE
  scanned = false;
  return -1;
  //  Serial.println("DASDAFHSFHDFJFGNSDJFSD");



}

void printCredits(int index) { //Also does LCD print of username and credit values using string buffers
  String user;
  String credits;
  int newPos;
  users_file = SD.open("Users.csv"); // OPEN FILE
  if (!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  int n = 0; // to put in all chars into char array
  //        char data[users_file.size()];
  char c;
  while (users_file.available()) //file read for USERNAME
  {
    users_file.seek(index);
    while ( c != ',')
    {
      c = users_file.read();
      if (c == ',' ) {
        break;
      }
      user += c;
      n++;
    }
    c = users_file.read(); //skip over comma
    nextPointer = users_file.position(); // position of credits saved to global variable
    break;
  }
  while (users_file.available()) //file read for CREDITS
  {
    while ( c != ',')
    {
      credits += c;
      c = users_file.read();
      n++;
    }
    break;
  }
  Serial.print("User: ");
  Serial.println(user);
  Serial.print("Credits: ");
  Serial.println(credits.toInt());
  lcd.setCursor(0, 0); // print to display
  lcd.print("        "); //first clear
  lcd.setCursor(0, 0);
  lcd.print("Hello ");
  lcd.println(user + "!    ");
  lcd.setCursor(0, 1);
  lcd.print("        "); // eliminates rouge characters on lcd "artifacts"
  lcd.setCursor(0, 1);
  lcd.print("Credits: ");
  lcd.print(credits.toInt());
  lcd.println("     "); // removing artifacts on lcd
  userCredits = credits.toInt(); // save to global variable as int
  users_file.close();
}


void lowerCredits(int index) {
  users_file = SD.open("Users.csv", FILE_WRITE); // OPEN FILE as write
  if (!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  while (1) //file read for USERNAME
  {
    users_file.seek(index - 1);
    char buf[4]; // temp location for decemented credit value
    String((userCredits - 1)).toCharArray(buf, 4); //filling buffer with decremented value
    int buflen = (String(userCredits)).length();
    if (buflen == 1) {
      buflen = 2;
      Serial.println("BUFLEN CHANGE");
    }
    users_file.write(buf, buflen ); // stable at 2, check 3
    lcd.setCursor(0, 1);
    lcd.print("        ");
    lcd.setCursor(0, 1);
    lcd.print("Credits: ");
    lcd.print(buf);
    lcd.print("   "); // erase artifacts
    delay(1400); //DELAY
    lcd.setCursor(0, 0);
    lcd.print("Thank You!      ");
    //    active = false;
    break;
  }
  users_file.close();
  // REPRINT TO MAIN SPLASH SCREEN
  lcd.setCursor(0, 0);
  lcd.print("||SodaBot v1.0||");
  lcd.setCursor(0, 1);
  lcd.print("Tap ur BuzzCard");
  delay(1000); // delay again
}

void dispenseCan(Servo myservo) { //attach servo before!
  int pos = 100;
  for (pos = 0; pos <= 100; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(30);                       // waits 15ms for the servo to reach the position
  }
  delay(200); // wait while dispenser is at lowest point 
  for (pos = 100; pos >= 5; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
//  myservo.detach();
}

