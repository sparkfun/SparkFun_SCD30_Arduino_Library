/*
  Reading CO2, humidity and temperature from the SCD30
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15112

  This example turns off the auto-calibrate function during .begin()

  Hardware Connections:
  Attach RedBoard to computer using a USB cable.
  Connect SCD30 to RedBoard using Qwiic cable.
  Open Serial Monitor at 115200 baud.
*/

#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;

void setup()
{
    Serial.begin(115200);
    Serial.println("SCD30 Example");
    Wire.begin();

    //Start sensor using the Wire port, but disable the auto-calibration
    if (airSensor.begin(Wire, false) == false)
    {
        Serial.println("Air sensor not detected. Please check wiring. Freezing...");
        while (1)
            ;
    }

    Serial.print("Auto calibration set to ");
    if (airSensor.getAutoSelfCalibration() == true)
        Serial.println("true");
    else
        Serial.println("false");

    //The SCD30 has data ready every two seconds
}

void loop()
{
    if (airSensor.dataAvailable())
    {
        Serial.print("co2(ppm):");
        Serial.print(airSensor.getCO2());

        Serial.print(" temp(C):");
        Serial.print(airSensor.getTemperature(), 1);

        Serial.print(" humidity(%):");
        Serial.print(airSensor.getHumidity(), 1);

        Serial.println();
    }
    else
        Serial.println("Waiting for new data");

    delay(500);
}