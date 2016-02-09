#include <CardReader.h>
#include <SPI.h>
#include <SD.h>

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
const int clock_pin = 0;
const int data_pin = 1;

//Create RFID reader object
CardReader reader(clock_pin, data_pin);

void setup()
{
  Serial.begin(9600);
  //Inorder to make the SD card library work, pin 10 must be set as an
  //output, even though we are using pin 8 as the chip select (CS)
  pinMode(10, OUTPUT);
  //Set the chip_select pin to output
  pinMode(chip_select, OUTPUT);
  //Attach the RFID reader as an interrupt
  attachInterrupt(1, readBit, RISING);
  //Try to initialize the SD Card
  if(!SD.begin(chip_select))
  {
    Serial.println("SD Card Initialization Failed");
    return;
  }
  Serial.println("SD Card Succesfully Initialized");
  //Open SD Card users file
  users_file = SD.open("users.txt");
  if(!users_file)
  {
    Serial.println("Users file was not able to be opened");
  }
  String file_in = ""
  char *tmp;
  int i = 0;
  if(users_file)
  {
    while(users_file.available())
    {
      file_in.concat(users_file.read())
    }
  }
  // tmp = strtok(file_in, ",");
  // while(tmp)
  // {
  //   ArrayKey[i++] = atoi()
  // }
}

void loop()
{

}

void readBit()
{
  reader.readBit();
}
