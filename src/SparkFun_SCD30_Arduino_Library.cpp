/*
  This is a library written for the SCD30
  SparkFun sells these at its website: www.sparkfun.com
  Do you like this library? Help support SparkFun. Buy a board!
  https://www.sparkfun.com/products/14751

  Written by Nathan Seidle @ SparkFun Electronics, May 22nd, 2018

  Updated February 1st 2021 to include some of the features of paulvha's version of the library
	(while maintaining backward-compatibility):
	https://github.com/paulvha/scd30
	Thank you Paul!

  The SCD30 measures CO2 with accuracy of +/- 30ppm.

  This library handles the initialization of the SCD30 and outputs
  CO2 levels, relative humidty, and temperature.

  https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.13

  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for more details.
*/

#include "SparkFun_SCD30_Arduino_Library.h"

SCD30::SCD30(void)
{
  // Constructor
}

//Initialize the Serial port
#ifdef USE_TEENSY3_I2C_LIB
bool SCD30::begin(i2c_t3 &wirePort, bool autoCalibrate, bool measBegin)
#else
bool SCD30::begin(TwoWire &wirePort, bool autoCalibrate, bool measBegin)
#endif
{
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  /* Especially during obtaining the ACK BIT after a byte sent the SCD30 is using clock stretching  (but NOT only there)!
   * The need for clock stretching is described in the Sensirion_CO2_Sensors_SCD30_Interface_Description.pdf
   *
   * The default clock stretch (maximum wait time) on the ESP8266-library (2.4.2) is 230us which is set during _i2cPort->begin();
   * In the current implementation of the ESP8266 I2C driver there is NO error message when this time expired, while
   * the clock stretch is still happening, causing uncontrolled behaviour of the hardware combination.
   *
   * To set ClockStretchlimit() a check for ESP8266 boards has been added in the driver.
   *
   * With setting to 20000, we set a max timeout of 20mS (> 20x the maximum measured) basically disabling the time-out
   * and now wait for clock stretch to be controlled by the client.
   */

#if defined(ARDUINO_ARCH_ESP8266)
  _i2cPort->setClockStretchLimit(200000);
#endif

  uint16_t fwVer;
  if (getFirmwareVersion(&fwVer) == false) // Read the firmware version. Return false if the CRC check fails.
    return (false);

  if (_printDebug == true)
  {
    _debugPort->print(F("SCD30 begin: got firmware version 0x"));
    _debugPort->println(fwVer, HEX);
  }

  if (measBegin == false) // Exit now if measBegin is false
    return (true);

  //Check for device to respond correctly
  if (beginMeasuring() == true) //Start continuous measurements
  {
    setMeasurementInterval(2);             //2 seconds between measurements
    setAutoSelfCalibration(autoCalibrate); //Enable auto-self-calibration

    return (true);
  }

  return (false); //Something went wrong
}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
void SCD30::enableDebugging(Stream &debugPort)
{
  _debugPort = &debugPort;
  _printDebug = true;
}

//Returns the latest available CO2 level
//If the current level has already been reported, trigger a new read
uint16_t SCD30::getCO2(void)
{
  if (co2HasBeenReported == true) //Trigger a new read
    readMeasurement();            //Pull in new co2, humidity, and temp into global vars

  co2HasBeenReported = true;

  return (uint16_t)co2; //Cut off decimal as co2 is 0 to 10,000
}

//Returns the latest available humidity
//If the current level has already been reported, trigger a new read
float SCD30::getHumidity(void)
{
  if (humidityHasBeenReported == true) //Trigger a new read
    readMeasurement();                 //Pull in new co2, humidity, and temp into global vars

  humidityHasBeenReported = true;

  return humidity;
}

//Returns the latest available temperature
//If the current level has already been reported, trigger a new read
float SCD30::getTemperature(void)
{
  if (temperatureHasBeenReported == true) //Trigger a new read
    readMeasurement();                    //Pull in new co2, humidity, and temp into global vars

  temperatureHasBeenReported = true;

  return temperature;
}

//Enables or disables the ASC
bool SCD30::setAutoSelfCalibration(bool enable)
{
  if (enable)
    return sendCommand(COMMAND_AUTOMATIC_SELF_CALIBRATION, 1); //Activate continuous ASC
  else
    return sendCommand(COMMAND_AUTOMATIC_SELF_CALIBRATION, 0); //Deactivate continuous ASC
}

//Set the forced recalibration factor. See 1.3.7.
//The reference CO2 concentration has to be within the range 400 ppm ≤ cref(CO2) ≤ 2000 ppm.
bool SCD30::setForcedRecalibrationFactor(uint16_t concentration)
{
  if (concentration < 400 || concentration > 2000)
  {
    return false; //Error check.
  }
  return sendCommand(COMMAND_SET_FORCED_RECALIBRATION_FACTOR, concentration);
}

//Get the temperature offset. See 1.3.8.
float SCD30::getTemperatureOffset(void)
{
  uint16_t response = readRegister(COMMAND_SET_TEMPERATURE_OFFSET);

  union
  {
    int16_t signed16;
    uint16_t unsigned16;
  } signedUnsigned; // Avoid any ambiguity casting int16_t to uint16_t
  signedUnsigned.signed16 = response;

  return (((float)signedUnsigned.signed16) / 100.0);
}

//Set the temperature offset to remove module heating from temp reading
bool SCD30::setTemperatureOffset(float tempOffset)
{
  //Temp offset is only positive. See: https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/issues/27#issuecomment-971986826
  //"The SCD30 offset temperature is obtained by subtracting the reference temperature from the SCD30 output temperature"
  //https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Low_Power_Mode.pdf

  if(tempOffset < 0.0) return(false);

  uint16_t value = tempOffset * 100;

  return sendCommand(COMMAND_SET_TEMPERATURE_OFFSET, value);
}

//Get the altitude compenstation. See 1.3.9.
uint16_t SCD30::getAltitudeCompensation(void)
{
  return readRegister(COMMAND_SET_ALTITUDE_COMPENSATION);
}

//Set the altitude compenstation. See 1.3.9.
bool SCD30::setAltitudeCompensation(uint16_t altitude)
{
  return sendCommand(COMMAND_SET_ALTITUDE_COMPENSATION, altitude);
}

//Set the pressure compenstation. This is passed during measurement startup.
//mbar can be 700 to 1200
bool SCD30::setAmbientPressure(uint16_t pressure_mbar)
{
  if (pressure_mbar < 700 || pressure_mbar > 1200)
  {
    return false;
  }
  return sendCommand(COMMAND_CONTINUOUS_MEASUREMENT, pressure_mbar);
}

// SCD30 soft reset
void SCD30::reset()
{
  sendCommand(COMMAND_RESET);
}

// Get the current ASC setting
bool SCD30::getAutoSelfCalibration()
{
  uint16_t response = readRegister(COMMAND_AUTOMATIC_SELF_CALIBRATION);
  if (response == 1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

//Begins continuous measurements
//Continuous measurement status is saved in non-volatile memory. When the sensor
//is powered down while continuous measurement mode is active SCD30 will measure
//continuously after repowering without sending the measurement command.
//Returns true if successful
bool SCD30::beginMeasuring(uint16_t pressureOffset)
{
  return (sendCommand(COMMAND_CONTINUOUS_MEASUREMENT, pressureOffset));
}

//Overload - no pressureOffset
bool SCD30::beginMeasuring(void)
{
  return (beginMeasuring(0));
}

// Stop continuous measurement
bool SCD30::StopMeasurement(void)
{
  return (sendCommand(COMMAND_STOP_MEAS));
}

//Sets interval between measurements
//2 seconds to 1800 seconds (30 minutes)
bool SCD30::setMeasurementInterval(uint16_t interval)
{
  return sendCommand(COMMAND_SET_MEASUREMENT_INTERVAL, interval);
}

//Gets interval between measurements
//2 seconds to 1800 seconds (30 minutes)
uint16_t SCD30::getMeasurementInterval(void)
{
  uint16_t interval = 0;
  getSettingValue(COMMAND_SET_MEASUREMENT_INTERVAL, &interval);
  return (interval);
}

//Returns true when data is available
bool SCD30::dataAvailable()
{
  uint16_t response = readRegister(COMMAND_GET_DATA_READY);

  if (response == 1)
    return (true);
  return (false);
}

//Get 18 bytes from SCD30
//Updates global variables with floats
//Returns true if success
bool SCD30::readMeasurement()
{
  //Verify we have data from the sensor
  if (dataAvailable() == false)
    return (false);

  ByteToFl tempCO2;
  tempCO2.value = 0;
  ByteToFl tempHumidity;
  tempHumidity.value = 0;
  ByteToFl tempTemperature;
  tempTemperature.value = 0;

  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(COMMAND_READ_MEASUREMENT >> 8);   //MSB
  _i2cPort->write(COMMAND_READ_MEASUREMENT & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (0); //Sensor did not ACK

  delay(3);

  const uint8_t receivedBytes = _i2cPort->requestFrom((uint8_t)SCD30_ADDRESS, (uint8_t)18);
  bool error = false;
  if (_i2cPort->available())
  {
    byte bytesToCrc[2];
    for (byte x = 0; x < 18; x++)
    {
      byte incoming = _i2cPort->read();

      switch (x)
      {
      case 0:
      case 1:
      case 3:
      case 4:
        tempCO2.array[x < 3 ? 3 - x : 4 - x] = incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      case 6:
      case 7:
      case 9:
      case 10:
        tempTemperature.array[x < 9 ? 9 - x : 10 - x] = incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      case 12:
      case 13:
      case 15:
      case 16:
        tempHumidity.array[x < 15 ? 15 - x : 16 - x] = incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      default:
        //Validate CRC
        uint8_t foundCrc = computeCRC8(bytesToCrc, 2);
        if (foundCrc != incoming)
        {
          if (_printDebug == true)
          {
            _debugPort->print(F("readMeasurement: found CRC in byte "));
            _debugPort->print(x);
            _debugPort->print(F(", expected 0x"));
            _debugPort->print(foundCrc, HEX);
            _debugPort->print(F(", got 0x"));
            _debugPort->println(incoming, HEX);
          }
          error = true;
        }
        break;
      }
    }
  }
  else
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readMeasurement: no SCD30 data found from I2C, i2c claims we should receive "));
      _debugPort->print(receivedBytes);
      _debugPort->println(F(" bytes"));
    }
    return false;
  }

  if (error)
  {
    if (_printDebug == true)
      _debugPort->println(F("readMeasurement: encountered error reading SCD30 data."));
    return false;
  }
  //Now copy the uint32s into their associated floats
  co2 = tempCO2.value;
  temperature = tempTemperature.value;
  humidity = tempHumidity.value;

  //Mark our global variables as fresh
  co2HasBeenReported = false;
  humidityHasBeenReported = false;
  temperatureHasBeenReported = false;

  return (true); //Success! New data available in globals.
}

//Gets a setting by reading the appropriate register.
//Returns true if the CRC is valid.
bool SCD30::getSettingValue(uint16_t registerAddress, uint16_t *val)
{
  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(registerAddress >> 8);   //MSB
  _i2cPort->write(registerAddress & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  delay(3);

  _i2cPort->requestFrom((uint8_t)SCD30_ADDRESS, (uint8_t)3); // Request data and CRC
  if (_i2cPort->available())
  {
    uint8_t data[2];
    data[0] = _i2cPort->read();
    data[1] = _i2cPort->read();
    uint8_t crc = _i2cPort->read();
    *val = (uint16_t)data[0] << 8 | data[1];
    uint8_t expectedCRC = computeCRC8(data, 2);
    if (crc == expectedCRC) // Return true if CRC check is OK
      return (true);
    if (_printDebug == true)
    {
      _debugPort->print(F("getSettingValue: CRC fail: expected 0x"));
      _debugPort->print(expectedCRC, HEX);
      _debugPort->print(F(", got 0x"));
      _debugPort->println(crc, HEX);
    }
  }
  return (false);
}

//Gets two bytes from SCD30
uint16_t SCD30::readRegister(uint16_t registerAddress)
{
  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(registerAddress >> 8);   //MSB
  _i2cPort->write(registerAddress & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (0); //Sensor did not ACK

  delay(3);

  _i2cPort->requestFrom((uint8_t)SCD30_ADDRESS, (uint8_t)2);
  if (_i2cPort->available())
  {
    uint8_t msb = _i2cPort->read();
    uint8_t lsb = _i2cPort->read();
    return ((uint16_t)msb << 8 | lsb);
  }
  return (0); //Sensor did not respond
}

//Sends a command along with arguments and CRC
bool SCD30::sendCommand(uint16_t command, uint16_t arguments)
{
  uint8_t data[2];
  data[0] = arguments >> 8;
  data[1] = arguments & 0xFF;
  uint8_t crc = computeCRC8(data, 2); //Calc CRC on the arguments only, not the command

  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(command >> 8);     //MSB
  _i2cPort->write(command & 0xFF);   //LSB
  _i2cPort->write(arguments >> 8);   //MSB
  _i2cPort->write(arguments & 0xFF); //LSB
  _i2cPort->write(crc);
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  return (true);
}

//Sends just a command, no arguments, no CRC
bool SCD30::sendCommand(uint16_t command)
{
  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(command >> 8);   //MSB
  _i2cPort->write(command & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (false); //Sensor did not ACK

  return (true);
}

//Given an array and a number of bytes, this calculate CRC8 for those bytes
//CRC is only calc'd on the data portion (two bytes) of the four bytes being sent
//From: http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
//Tested with: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
//x^8+x^5+x^4+1 = 0x31
uint8_t SCD30::computeCRC8(uint8_t data[], uint8_t len)
{
  uint8_t crc = 0xFF; //Init with 0xFF

  for (uint8_t x = 0; x < len; x++)
  {
    crc ^= data[x]; // XOR-in the next input byte

    for (uint8_t i = 0; i < 8; i++)
    {
      if ((crc & 0x80) != 0)
        crc = (uint8_t)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }

  return crc; //No output reflection
}
