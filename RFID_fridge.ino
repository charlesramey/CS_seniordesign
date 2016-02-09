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

//Define which pin is used for SD card chip select
const int chip_select = 8;

//Define where the RFID reader is plugged in
const int clock_pin = 0;
const int data_pin = 1;

//Create RFID reader object
CardReader reader(clock_pin, data_pin);

void setup() {
  Serial.begin(9600);
  //Inorder to make the SD card library work, pin 10 must be set as an
  //output, even though we are using pin 8 as the chip select (CS)
  pinMode(10, OUTPUT);
  //Set the chip_select pin to output
  pinMode(chip_select, OUTPUT);
  attachInterrupt(1, readBit, RISING);
}

void loop() {
  
}

void readBit() { reader.readBit(); }
