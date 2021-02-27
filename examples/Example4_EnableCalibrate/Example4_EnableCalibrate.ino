/*
  Reading CO2, humidity and temperature from the SCD30
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 22nd, 2018
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15112

  This example turns on the auto-calibrate function during .begin()
  Please see section 1.3.6 of the SCD30 datasheet:

  "When activated for the first time a
    period of minimum 7 days is needed so that the algorithm can find its initial parameter set for ASC. The sensor has to be exposed
    to fresh air for at least 1 hour every day. Also during that period, the sensor may not be disconnected from the power supply,
    otherwise the procedure to find calibration parameters is aborted and has to be restarted from the beginning. The successfully
    calculated parameters are stored in non-volatile memory of the SCD30 having the effect that after a restart the previously found
    parameters for ASC are still present. "

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

    //Start sensor using the Wire port and enable the auto-calibration (ASC)
    if (airSensor.begin(Wire, true) == false)
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
