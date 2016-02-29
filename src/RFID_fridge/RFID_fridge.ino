#include "Wiegand.h"
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>

/* SD Card Hook Up Guide
 *  MOSI -> Pin 11
 *  MISO -> Pin 12
 *  CLK -> Pin 13
 *  CS -> 8
 */

//Requisite SD card variables
Sd2Card card;
SdVolume volume;
SdFile root;

//Users file from SD Card
File users_file;
const int numUsers = 30;  // SET NUMBER MAX USERS 
long cards[numUsers];
String users[numUsers];
int credits[numUsers];

//Define which pin is used for SD card chip select
const int chip_select = 8;

//Define where the RFID reader is plugged in
const int clock_pin = 3;
const int data_pin = 2;

//Card ID 
long Card_ID; //type long for long int

//Create RFID reader object
Wiegand reader;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

void setup()
{
  Serial.begin(9600);
  reader.begin();
  // Attach the 0-bit Wiegand signal to Arduino's Interrupt 0 (Pin 2 for UNO)
  // Attach the 1-bit Wiegand signal to Arduino's Interrupt 1 (Pin 3 for UNO)
  reader.attach(0,1);
  //Inorder to make the SD card library work, pin 10 must be set as an
  //output, even though we are using pin 8 as the chip select (CS)
  pinMode(10, OUTPUT);
  digitalWrite(10,HIGH); // For the backlight to stay on! 
  //Set the chip_select pin to output fo SD reader
  pinMode(chip_select, OUTPUT);

  //Try to initialize the SD Card
  if(!SD.begin(chip_select))
  {
    Serial.println("SD Card Initialization Failed");
    return;
  }
  Serial.println("SD Card Succesfully Initialized");
  //Open SD Card users file
  users_file = SD.open("Users.csv");
  if(!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  
  int n = 0; // to put in all chars into char array
  char data[users_file.size()];
  while (users_file.available()) //file read
  {
    data[n] = users_file.read(); //all chars are inside char array
    n++;
  }
  // now lines will be parsed and placed into arrays
  int a = 1; // to track which array to place items
  int b = -1; //to keep count iterating
  char *p = data;
  char *line; // going through line by line
  char *entity;
  while (( line = strtok_r(p,"\n",&p)) != NULL){ // loop skips first row of headers
    while (( entity = strtok_r(line,",",&line)) != NULL){
      if ( a == 7 ){
        a = 4;
        }
      if ( a == 4 ){
        cards[b] = ((String)entity).toInt(); //needs casting because pointers 
        }
      if ( a == 5){
        users[b] = (String)entity;
        }
      if ( a == 6 ){
        credits[b] = ((String)entity).toInt();
        }     
      a++;
      }
      b++;
      }
//////This block shows first few values of arrays      
//Serial.print(cards[0]);
//Serial.print(" ");
//Serial.print(cards[1]);
//Serial.print(" ");
//Serial.println(cards[2]);
//Serial.print(users[0]);
//Serial.print(" ");
//Serial.print(users[1]);
//Serial.print(" ");
//Serial.println(users[2]);
//Serial.print(credits[0]);
//Serial.print(" ");
//Serial.print(credits[1]);
//Serial.print(" ");
//Serial.println(credits[2]);
 
//  Serial.println("out of loop");
//  String file_in = "";
//  char *tmp;
//  int i = 0;
//  if(users_file)
//  {
//    while(users_file.available())
//    {
//      file_in.concat(users_file.read());
//    }
//  }
  // tmp = strtok(file_in, ",");
  // while(tmp)
  // {
  //   ArrayKey[i++] = atoi()
  // }

  
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Testing");
}


boolean found = false;
void loop()
{ 
  if(reader.available()==true)
  {
    Card_ID = reader.getCardCode();
    Serial.print(" Facility Code = ");
    Serial.print(reader.getFacilityCode(),DEC);
    Serial.print(", Card Code = ");
    Serial.println(Card_ID,DEC);
    
    for (int i = 0; i < numUsers; ++i){
      if ((cards[i]) == Card_ID){
        Serial.println("SUCESESS");
        found = true;
        lcd.setCursor(0,0);
        lcd.print("User: ");
        lcd.print(users[i]);
        lcd.setCursor(0,1);
        lcd.print("Credits: ");
        lcd.print(credits[i]);
        }
//      else { Serial.println("no");}  
      }
      if (!found)
        Serial.println("no");
    reader.reset();
  }
}


