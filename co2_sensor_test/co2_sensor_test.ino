/*
MHZ19 libary example 'BasicUsage'

https://github.com/WifWaf/MH-Z19/blob/master/examples/BasicUsage/BasicUsage.ino

if autoCalibration is turned off, do a manual calibration from time to time by pulling the zero HD low (0V) for 7 Secs as per the datasheet.
*/

#include <MHZ19.h>                                        
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial

// --> rx/tx wires need to be crossed!
#define RX_PIN 11                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 12                                          // Tx pin which the MHZ19 Rx pin is attached to


MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

unsigned long getDataTimer = 0;

void setup() {
    Serial.begin(9600);                                     // Device to serial monitor feedback

    mySerial.begin(9600);                                   // Device to MH-Z19 Serial baudrate (should not be changed)
    myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 

    myMHZ19.autoCalibration(false);                              // Turn auto calibration ON (OFF autoCalibration(false))
}

void loop() {
    if (millis() - getDataTimer >= 2000) {
        int CO2; 

        /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
        if below background CO2 levels or above range (useful to validate sensor). You can use the 
        usual documented command with getCO2(false) */

        CO2 = myMHZ19.getCO2(false);                             // Request CO2 (as ppm)
        
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                

        int8_t Temp;
        Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);                               

        getDataTimer = millis();
    }
}
