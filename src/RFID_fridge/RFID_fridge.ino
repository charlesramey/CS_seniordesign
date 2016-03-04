#include "Wiegand.h"
#include <SD.h>
#include <LiquidCrystal.h>
#include <stdio.h>
#include <string.h>

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
int lastCardID = -1;

//Define which pin is used for SD card chip select
const int chip_select = 8;

//Define where the RFID reader is plugged in
const int clock_pin = 3;
const int data_pin = 2;

//Card ID 
long Card_ID; //type long for long int
int dispense = 1; // value of one soda

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

  // now lines will be parsed and placed into arrays

  lcd.begin(16, 2);
  lcd.setCursor(0,0);
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
  
  if(reader.available()==true)
  {
    active = true;
    scanned = true;
    Card_ID = reader.getCardCode();
    Serial.print(" Facility Code = ");
    Serial.print(reader.getFacilityCode(),DEC);
    Serial.print(", Card Code = ");
    Serial.println(Card_ID,DEC);

    lcd.setCursor(0,1);
    lcd.print(getCredits(Card_ID));

    reader.reset();
   }

   


    while (active){



        lcd_key = read_LCD_buttons(); // read buttons 
        lcd.setCursor(0,1);  
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
            lcd.print("UP    ");
            break;
            }
          case btnDOWN:
            {

      //      lcd.print("DOWN  ");
            while (dispense > 0){ //because button press results in many calls 
              pay = true;
              dispense--;
              Serial.println("DOWN");    
              Serial.println(lowerCredits(Card_ID));
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

    int getCredits (long Card_ID){

        Serial.println(Card_ID);
        users_file = SD.open("Users.csv"); // OPEN FILE
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
         users_file.close(); // CLOSE FILE  
         scanned = false;
         Serial.println("DASDAFHSFHDFJFGNSDJFSD");
      
      int a = 1; // to track which array to place items
      int b = -1; //to keep count iterating
      char *p = data;
      char *line; // going through line by line
      char *entity;
      if(onetime){
        Serial.println(data);
        onetime=false;
        } 
      while (( line = strtok_r(p,"\n",&p)) != NULL){ // loop skips first row of headers
        while (( entity = strtok_r(NULL,",",&p)) != NULL){
          if (anothertime){
//            Serial.println(p);
            anothertime= false;
            }
//            Serial.println(entity);
          if ( a == 5 ){
           a = 1;
            }
          if ( a == 1 ){
             if (Card_ID == ((String)entity).toInt()){
                lastCardID = Card_ID;
                Serial.println("SUCCESSSSSSSS");
                found = true;
              }
           }
          if ( a == 2 && found){
              lcd.setCursor(0,0);
              lcd.print((String)entity + " ");
            }
          if ( a == 3 && found){
              lcd.setCursor(0,1);
//              lcd.print((String)entity);
//              Serial.print("7 conditional: ");
//              Serial.println((String)entity);
              return ((String)entity).toInt();
//              if (pay){
////                strncpy(entity,(char*)(((String)entity).toInt() - 1),2);
//                Serial.println((((String)entity).toInt() - 1));
////                entity = (char*)(((String)entity).toInt() - 1);
//                lcd.setCursor(0,1);
//                lcd.print((String)p);
////                Serial.println((String)entity);
//                pay = false;
//                }
//            credits[b] = ((String)entity).toInt();
            }   
          if ( a == 4 ){
//            admins[b] = ((String)entity).toInt();
            }     
          a++;
          }
          b++;
          if (!found){
            return -1;
            }
          
          found = false;
          }
      
      
      }



       int lowerCredits (long Card_ID){

        Serial.println(Card_ID);
        users_file = SD.open("Users.csv"); // OPEN FILE
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
         users_file.close(); // CLOSE FILE  
         scanned = false;
         Serial.println("DASDAFHSFHDFJFGNSDJFSD");
      
      int a = 1; // to track which array to place items
      int b = -1; //to keep count iterating
      char *p = data;
      char *line; // going through line by line
      char *entity;
      if(onetime){
        Serial.println(data);
        onetime=false;
        } 
      while (( line = strtok_r(p,"\n",&p)) != NULL){ // loop skips first row of headers
        while (( entity = strtok_r(NULL,",",&p)) != NULL){
          if (anothertime){
//            Serial.println(p);
            anothertime= false;
            }
//            Serial.println(entity);
          if ( a == 5 ){
           a = 1;
            }
          if ( a == 1 ){ //card IDs
             if (Card_ID == ((String)entity).toInt()){
                lastCardID = Card_ID;
                Serial.println("SUCCESSSSSSSS");
                found = true;
              }
           }
          if ( a == 2 && found){ //user name
              lcd.setCursor(0,0);
              lcd.print((String)entity + " ");
            }
          if ( a == 3 && found){ //credits
              lcd.setCursor(0,1);
//              lcd.print((String)entity);
//              Serial.print("7 conditional: ");
//              Serial.println((String)entity);
                Serial.print("Enter: ");
                Serial.println(entity);
                Serial.println(  ((int)(((String)entity).toInt()) ) - 1 );
                entity = (char*)(((int)(((String)entity).toInt()) ) - 1 );            
                Serial.println(entity);
                  
              
//                users_file = SD.open("Users.csv", FILE_WRITE); // OPEN FILE as write
//                if(!users_file)
//                {
//                  Serial.println("Users file was not able to be opened in decrement");
//                }
//                int n = 0; // to put in all chars into char array
////                char data[users_file.size()];
////                while (users_file.available()) //file read
////                {
////                data[n] = users_file.read(); //all chars are inside char array
////                n++;
////                }
//                
//
//                users_file.close(); // CLOSE FILE 


                


                
              return ((String)entity).toInt();
//              if (pay){
////                strncpy(entity,(char*)(((String)entity).toInt() - 1),2);
//                Serial.println((((String)entity).toInt() - 1));
////                entity = (char*)(((String)entity).toInt() - 1);
//                lcd.setCursor(0,1);
//                lcd.print((String)p);
////                Serial.println((String)entity);
//                pay = false;
//                }
//            credits[b] = ((String)entity).toInt();
            }   
          if ( a == 4 ){ //admin
//            admins[b] = ((String)entity).toInt();
            }     
          a++;
          }
          b++;
          if (!found){
            return -1;
            }
          
          found = false;
          }
      
      
      }


      


