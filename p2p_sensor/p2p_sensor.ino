/*
Copied from https://github.com/Apress/begin-lora-radio-networks-with-arduino/blob/master/Chapter%204/sensor.ino, which corresponds to the code published 
in the book "Beginning LoRa Radio Networks with Arduino: Build Long Range, Low Power Wireless IoT Networks by Pradeeka Seneviratne (Apress, 2019)".

Temp messure DS18B20: https://www.youtube.com/watch?v=UY4zOXchK4w
1-Wire Protocol: https://playground.arduino.cc/Learning/OneWire-DE/

co2: https://learn.adafruit.com/adafruit-sgp30-gas-tvoc-eco2-mox-sensor/arduino-code

*/
#include <DallasTemperature.h>
#include <Adafruit_SGP30.h>
#include <OneWire.h>
#include <RH_RF95.h>
#include <EEPROM.h>

//Temp sensor pins
#define oneWireBusPin 5  // define the 1 wire bus pin
#define virtualVCC 4     // a virtual +5V pin
#define virtualGND 3     // a virtual GND pin

//Radio pinout setup
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

//Radio frequency setup
#define RF95_FREQ 868.0

//Radio init
RH_RF95 rf95(RFM95_CS, RFM95_INT);

//Temp sensor init
OneWire oneWireBus(oneWireBusPin);
DallasTemperature temperatureSensor(&oneWireBus);
DeviceAddress temperatureSensorAddress;
int connectedDevicesNomber = 0;
float currentTemperature = 0.0;

//CO2 sensor init
Adafruit_SGP30 sgp;


char deviceIdString[4] = "d";  //number will be read and appended from EEPROM



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

  Serial.begin(9600);
  delay(100);

  // read/generate deviceIdString
  char buf[3];
  int device_identifier;
  EEPROM.get(0, device_identifier);
  itoa(device_identifier, buf, 10);  // convert int to str
  strcat(deviceIdString, buf);       // append buf to deviceIdString

  Serial.print("device_identifier: '");
  Serial.print(deviceIdString);
  Serial.println("'");


  if (!sgp.begin()) {
    Serial.println("CO2 sensor not found");
    //while (1);
  }


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

  char co2strBuffer[4];  // create buffer

  if (sgp.IAQmeasure()) {
    // ----------- TODO: make multiple messurements and sent average -----------
    itoa(sgp.eCO2, co2strBuffer, 10);  // convert int to str
  } else {
    itoa(-1, co2strBuffer, 10);
  }





  temperatureSensor.requestTemperaturesByAddress(temperatureSensorAddress);   // command for a device with a specific address to perform a temperature conversion (to read the temperature)
  currentTemperature = temperatureSensor.getTempC(temperatureSensorAddress);  // Get temperature for device index in C (Slow process)

  char radiopacket[20] = "";                              // create radiopacket initialized with device identifier
  char temperature_buffer[6];                             // create temperature_buffer
  dtostrf(currentTemperature, 5, 2, temperature_buffer);  // write formated float to temperature_buffer

  strcat(radiopacket, deviceIdString);      // append deviceIdString to radiopacket
  strcat(radiopacket, "|");                 // append string to radiopacket
  strcat(radiopacket, temperature_buffer);  // append temperature to radiopacket
  strcat(radiopacket, "|");                 // append string to radiopacket
  strcat(radiopacket, co2strBuffer);        // append co2 str to radiopacket



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
