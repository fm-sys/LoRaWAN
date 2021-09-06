//**********************************************************************************************************//
// Author: 
// WellTronic
//
// Description:
// This code is part of a video series covering all Arduino sensors from the Arduino sensor kit.
// One of the sensors in this video series is the DS18B20 temperature sensor.
//
// In this video I will explain step by step how to use the DS18B20 temperature sensor.
// https://www.youtube.com/watch?v=UY4zOXchK4w
//
// You're also welcome to take a look at the YouTube channel for more details about hardware and software.
// https://www.youtube.com/channel/UC0UCNqE8i4unG8nfuakd0vw
// 
// Enjoy working with this sensor and see you soon :) !
//
//**********************************************************************************************************//

#include <DallasTemperature.h>
#include <OneWire.h>

float currentTemperature = 0.0;                                                           // define the current temperature variable
int oneWireBusPin = 13;                                                                    // define the 1 wire bus pin
OneWire oneWireBus(oneWireBusPin);                                                        // create an instance of the OneWire Library and define the oneWireBusPin in it 
DallasTemperature temperatureSensor(&oneWireBus);                                         // create an instance of the Dallastemperature Library and define the OneWire instance oneWireBus in it
DeviceAddress temperatureSensorAddress;                                                   // temperatureDeviceAddress is an uint8_t array [8]
int connectedDevicesNomber = 0;

void setup() {
Serial.begin(9600);                                                                       // start the serial monitor
Serial.println("Welcome to WellTronic's temperature sensor tutorial");                    // print the text "Welcome to WellTronic's temperature sensor tutorial" into the serial monitor        
  
temperatureSensor.begin();                                                                // start the bus 

connectedDevicesNomber = temperatureSensor.getDeviceCount();                              // returns the number of devices found on the bus
  
temperatureSensor.getAddress(temperatureSensorAddress, 0);                                // get the address of the first device connected to the bus 
printDeviceAddress(temperatureSensorAddress);                                             // print this address
   
                                                                                          
if (temperatureSensor.setResolution(temperatureSensorAddress,12, false) == true){         // set resolution of a device to 9, 10, 11, or 12 bits. If new resolution is out of range, 9 bits is used.
  Serial.print("Resolution successfully set to ");
  Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));                // get the resolution of a device with a given address
  Serial.println(" bit");
}
else {
  Serial.println("Resolution configuration failed ");
  Serial.print("Resolution automatically set to ");
  Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));                 // get the resolution of a device with a given address
  Serial.println(" bit");
}

temperatureSensor.setLowAlarmTemp(temperatureSensorAddress,20);                            // set the lower alarm limit
temperatureSensor.setHighAlarmTemp(temperatureSensorAddress,28);                           // set the upper alarm limit

Serial.print("Alarm lower limit: ");
Serial.println(temperatureSensor.getLowAlarmTemp(temperatureSensorAddress));               // get the lower alarm limit
Serial.print("Alarm upper limit: ");
Serial.println(temperatureSensor.getHighAlarmTemp(temperatureSensorAddress));              // get the upper alarm limit
}

void loop() {
temperatureSensor.requestTemperaturesByAddress(temperatureSensorAddress);                  // command for a device with a specific address to perform a temperature conversion (to read the temperature)
currentTemperature = temperatureSensor.getTempC(temperatureSensorAddress);                 // Get temperature for device index in C (Slow process)                                                               
 Serial.print("Temperature: ");
 Serial.print(currentTemperature);
 Serial.print(" C");
 Serial.print("   |  ");
 currentTemperature = temperatureSensor.getTempF(temperatureSensorAddress);                // Get temperature for device index in F (Slow process)
 Serial.print(currentTemperature);
 Serial.print(" F");
 Serial.print("   |  ");
 Serial.print("TemperatureAlarm: ");
 Serial.print(temperatureSensor.hasAlarm(temperatureSensorAddress));
 Serial.println("");

 delay(1000);
}

void printDeviceAddress(DeviceAddress deviceAddress){
  for (int i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}
