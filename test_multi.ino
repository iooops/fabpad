/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Arduino.h>
#include <BLEMidi.h>

#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// Define the I2C addresses for each MPR121
#define MPR121_ADDR_1 0x5A // Default address
#define MPR121_ADDR_2 0x5B // Address with ADDR pin tied to 3.3V

// Create instances of the MPR121 class for each sensor
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t last1touched = 0;
uint16_t curr1touched = 0;
uint16_t last2touched = 0;
uint16_t curr2touched = 0;

uint16_t keys[20] = {60, 60, 60, 60, 62, 62, 62, 62, 
64, 64, 64, 64, 67, 67, 67, 67, 69, 69, 69, 69};
uint16_t velocities[20] = {31, 63, 95, 127, 31, 63, 95, 127, 
31, 63, 95, 127, 31, 63, 95, 127, 31, 63, 95, 127};

void setup() {
  Serial.begin(9600);

  BLEMidiServer.begin("Basic MIDI device");

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  cap1.begin(MPR121_ADDR_1);
  cap2.begin(MPR121_ADDR_2);

  Serial.println("MPR121 found!");
}

void loop() {
  // Get the currently touched pads
  curr1touched = cap1.touched();
  curr2touched = cap2.touched();

  processTouch(curr1touched, 0, last1touched);
  processTouch(curr2touched, 12, last2touched);

  last1touched = curr1touched;
  last2touched = curr2touched;

  // comment out this line for detailed data from the sensor!
  return;
  
  // // debugging info, what
  // Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  // Serial.print("Filt: ");
  // for (uint8_t i=0; i<12; i++) {
  //   Serial.print(cap.filteredData(i)); Serial.print("\t");
  // }
  // Serial.println();
  // Serial.print("Base: ");
  // for (uint8_t i=0; i<12; i++) {
  //   Serial.print(cap.baselineData(i)); Serial.print("\t");
  // }
  // Serial.println();
  
  // // put a delay so it isn't overwhelming
  // delay(100);
}

void processTouch(uint16_t touched, uint8_t start, uint16_t last_touched) {
  bool ble_connected = BLEMidiServer.isConnected();
  for (uint8_t i = 0; i < 12; i++) {
    uint8_t ii = start + i;
    if ((touched & _BV(i)) && !(last_touched & _BV(i))) {
      Serial.print(ii); Serial.println(" touched");
      if (ble_connected) {
        BLEMidiServer.noteOn(0, keys[ii], velocities[ii]);
      }
    }
    if (!(touched & _BV(i)) && (last_touched & _BV(i)) ) {
      Serial.print(ii); Serial.println(" released");
      if (ble_connected) {
        BLEMidiServer.noteOff(0, keys[ii], velocities[ii]);
      }
    }
  }
}
