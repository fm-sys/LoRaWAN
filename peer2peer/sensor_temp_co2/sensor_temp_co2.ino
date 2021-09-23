/*
Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/sensor.ino, which corresponds to the code published 
in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

Temp messure DS18B20: https://www.youtube.com/watch?v=UY4zOXchK4w
1-Wire Protocol: https://playground.arduino.cc/Learning/OneWire-DE/

co2: https://learn.adafruit.com/adafruit-sgp30-gas-tvoc-eco2-mox-sensor/arduino-code

*/
#include <DallasTemperature.h>
#include <OneWire.h>
#include <RH_RF95.h>

//Radio pinout setup
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

#define RF95_FREQ 868.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

//temp messure pins
float currentTemperature = 0.0;
int oneWireBusPin = 5; // define the 1 wire bus pin
int virtualVCC = 4;
int virtualGND = 3;

OneWire oneWireBus(oneWireBusPin);                 // create an instance of the OneWire Library and define the oneWireBusPin in it
DallasTemperature temperatureSensor(&oneWireBus);  // create an instance of the Dallastemperature Library and define the OneWire instance oneWireBus in it
DeviceAddress temperatureSensorAddress;            // temperatureDeviceAddress is an uint8_t array [8]
int connectedDevicesNomber = 0;

void setup() {
  // misuse some digital pins so that we can avoid additional wiring
  // additional 5V power supply
  pinMode(virtualVCC, OUTPUT);
  digitalWrite(virtualVCC, HIGH);
  // additional GND pin
  pinMode(virtualGND, OUTPUT);
  digitalWrite(virtualGND, LOW);


  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial)
    ;
  Serial.begin(9600);
  delay(100);







  temperatureSensor.begin();  // start the bus
  connectedDevicesNomber = temperatureSensor.getDeviceCount();
  temperatureSensor.getAddress(temperatureSensorAddress, 0);
  printDeviceAddress(temperatureSensorAddress);


  if (temperatureSensor.setResolution(temperatureSensorAddress, 12, false) == true) {  // set resolution of a device to 9, 10, 11, or 12 bits. If new resolution is out of range, 9 bits is used.
    Serial.print("Temp resolution successfully set to ");
    Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));  // get the resolution of a device with a given address
    Serial.println(" bit");
  } else {
    Serial.println("Temp resolution configuration failed ");
    Serial.print("Temp resolution automatically set to ");
    Serial.print(temperatureSensor.getResolution(temperatureSensorAddress));  // get the resolution of a device with a given address
    Serial.println(" bit");
  }

  Serial.print("Temperature: ");
  Serial.print(temperatureSensor.getTempC(temperatureSensorAddress));
  Serial.println(" C");



  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("initializing…");
  }
  Serial.println("LoRa initialisation succeeded");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // This sets the power of the transciever. max is 23 apparantly.
  // Warning: setting Tx power to 20 runs the risk of making your chip very hot!  Datasheet cautions not to use a duty cycle of more than 1%.
  rf95.setTxPower(15, false);
}

void loop() {

  temperatureSensor.requestTemperaturesByAddress(temperatureSensorAddress);   // command for a device with a specific address to perform a temperature conversion (to read the temperature)
  currentTemperature = temperatureSensor.getTempC(temperatureSensorAddress);  // Get temperature for device index in C (Slow process)

  char radiopacket[20];                            // create buffer
  dtostrf(currentTemperature, 5, 2, radiopacket);  // write to buffer

  Serial.print("Send '");
  Serial.print(radiopacket);
  Serial.println("'");


  Serial.println("Sending...");
  delay(10);

  rf95.send((uint8_t *)radiopacket, 20);

  Serial.print("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  Serial.println("Done!");



  delay(10 * 1000);  // sec in ms
}

void printDeviceAddress(DeviceAddress deviceAddress) {
  for (int i = 0; i < 8; i++) {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}