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

//Define which pin is used for SD card chip select
const int chip_select = 8;

//Define where the RFID reader is plugged in
const int clock_pin = 3;
const int data_pin = 2;

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
//  if(!SD.begin(chip_select))
//  {
//    Serial.println("SD Card Initialization Failed");
//    return;
//  }
//  Serial.println("SD Card Succesfully Initialized");
  //Open SD Card users file
//  users_file = SD.open("users.txt");
//  if(!users_file)
//  {
//    Serial.println("Users file was not able to be opened");
//  }
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
}

void loop()
{
  
  lcd.setCursor(0,0);
  lcd.print("Testing");
  if(reader.available()==true)
  {
    Serial.print(" Facility Code = ");
    Serial.print(reader.getFacilityCode(),DEC);
    Serial.print(", Card Code = ");
    Serial.println(reader.getCardCode(),DEC);
    //print to lcd display on second line 
    lcd.setCursor(0,1);
    lcd.print(reader.getCardCode(), DEC);
    reader.reset();
  }
}

