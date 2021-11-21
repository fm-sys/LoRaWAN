/***
    eeprom_put example.
    
    Written by Christopher Andrews 2015
    Released under MIT licence.

    Modified by Florian Meyer 2021
***/

#include <EEPROM.h>

int device_identifier = 4;  //Variable to store in EEPROM.


void setup() {
  Serial.begin(9600);

  int eeAddress = 0;

  EEPROM.put(eeAddress, device_identifier);
  Serial.print("Device identifier 'd");
  Serial.print(device_identifier);
  Serial.println("' successfully written to EEPROM!");
}

void loop() {
  /* Empty loop */
}
