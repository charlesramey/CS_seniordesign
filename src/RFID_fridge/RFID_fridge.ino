#include "Wiegand.h"
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <string.h>

/* SD Card Hook Up Guide
    MOSI -> Pin 11
    MISO -> Pin 12
    CLK -> Pin 13
    CS -> 8
*/

//Requisite SD card variables
Sd2Card card;
SdVolume volume;
SdFile root;

//Users file from SD Card
File users_file;
int lastCardID = -1;

//Define which pin is used for SD card chip select
const int chip_select = 8;

//Define where the RFID reader is plugged in
const int clock_pin = 3;
const int data_pin = 2;

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
  lcd.print("Testing");
}


boolean readback = true; // if to read the csv again and update local array
boolean found = false; // if card ID is found
boolean scanned = false; // if a card has been scanned just then
boolean active = false; // if user has scanned card and may do something
boolean transaction = false; //if a transaction is occuring (decrement)
boolean pay = false; // if user selects can to be dispensed
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
    active = true;
    scanned = true;
    dispense = 1;
    Card_ID = reader.getCardCode();
    Serial.print(" Facility Code = ");
    Serial.print(reader.getFacilityCode(), DEC);
    Serial.print(", Card Code = ");
    Serial.println(Card_ID, DEC);

    lcd.setCursor(0, 1);
    int IDpointer = findID(Card_ID);
    Serial.print("IDP: ");
    Serial.println(IDpointer);
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
    lcd_key = read_LCD_buttons(); // read buttons
    lcd.setCursor(0, 1);
    switch (lcd_key)               // depending on which button was pushed, we perform an action
    {
      case btnRIGHT:
        {
          lcd.print("RIGHT ");

          break;
        }
      case btnLEFT:
        {
          lcd.print("LEFT   ");
          break;
        }
      case btnUP:
        {
          while (dispense > 0) { //because button press results in many calls
            pay = true;
            Serial.println("UP");
            raiseCredits(nextPointer); // Needs check for administrator
            dispense--;
            active = false;
          }
          break;
        }
      case btnDOWN:
        {

          //      lcd.print("DOWN  ");
          while (dispense > 0) { //because button press results in many calls
            if (userCredits == 0) {
              Serial.println("NO CREDITS");
              lcd.print("NO CREDITS");
              dispense--;
              active = false;
              break;
            }
            pay = true;
            Serial.println("DOWN");
            lowerCredits(nextPointer);
            dispense--;
            active = false;
          }
          //              transaction = false;
          break;
        }
      case btnSELECT:
        {
          lcd.print("SELECT");
          break;
        }
      case btnNONE:
        {
          //      lcd.print("NONE  ");
          break;
        }
    }
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
  Serial.println(credits);
  lcd.setCursor(0, 0); // print to display
  lcd.print("        "); //first clear
  lcd.setCursor(0, 0);
  lcd.print(user);
  lcd.setCursor(0, 1);
  lcd.print("        "); // eliminates rouge characters on lcd
  lcd.setCursor(0, 1);
  lcd.print(credits);
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
    Serial.print("BUF: ");
    Serial.println(buf);
    Serial.print("sizeof: ");
    int buflen = (String(userCredits)).length();
    Serial.println( buflen   );
    if (buflen == 1){
      buflen = 2; 
      Serial.println("BUFLEN CHANGE");
      }
    users_file.write(buf, buflen ); // stable at 2, check 3
    lcd.setCursor(0, 1);
    lcd.print("        ");    
    lcd.setCursor(0, 1);
    lcd.print(buf);
    //    active = false;
    break;
  }
  users_file.close();
}

void raiseCredits(int index) {
  users_file = SD.open("Users.csv", FILE_WRITE); // OPEN FILE as write
  if (!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  while (1) //file read for USERNAME
  {
    users_file.seek(index - 1);
    char buf[4]; // temp location for decemented credit value
    String((userCredits + 1)).toCharArray(buf, 4); //filling buffer
    Serial.print("BUF: ");
    Serial.println(buf);
    Serial.print("sizeof: ");
    int buflen = (String(userCredits)).length();
    Serial.println( buflen   );
    if (buflen == 1){
      buflen = 2;
      Serial.println("BUFLEN CHANGE"); 
      }
    users_file.write(buf, 2 ); // stable at 2, check 3
    lcd.setCursor(0, 1);
    lcd.print(buf);
    //    active = false;
    break;
  }
  users_file.close();
}
