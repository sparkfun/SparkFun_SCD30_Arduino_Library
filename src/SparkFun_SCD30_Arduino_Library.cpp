/*
  This is a library written for the SCD30
  SparkFun sells these at its website: www.sparkfun.com
  Do you like this library? Help support SparkFun. Buy a board!
  https://www.sparkfun.com/products/14751

  Written by Nathan Seidle @ SparkFun Electronics, May 22nd, 2018

  The SCD30 measures CO2 with accuracy of +/- 30ppm.

  This library handles the initialization of the SCD30 and outputs
  CO2 levels, relative humidty, and temperature.

  https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.5

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SparkFun_SCD30_Arduino_Library.h"

SCD30::SCD30(void)
{
  // Constructor
}

//Initialize the Serial port
#ifdef USE_TEENSY3_I2C_LIB
bool SCD30::begin(i2c_t3 &wirePort, bool autoCalibrate)
#else
bool SCD30::begin(TwoWire &wirePort, bool autoCalibrate)
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

  //Check for device to respond correctly
  if (beginMeasuring() == true) //Start continuous measurements
  {
    setMeasurementInterval(2);    //2 seconds between measurements
    setAutoSelfCalibration(autoCalibrate); //Enable auto-self-calibration

    return (true);
  }

  return (false); //Something went wrong
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
  return (float)response / 100;
}

//Set the temperature offset. See 1.3.8.
bool SCD30::setTemperatureOffset(float tempOffset)
{
  int16_t tickOffset = tempOffset * 100;
  return sendCommand(COMMAND_SET_TEMPERATURE_OFFSET, tickOffset);
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
  if (response == 1) {
    return true;
  }
  else {
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

//Sets interval between measurements
//2 seconds to 1800 seconds (30 minutes)
bool SCD30::setMeasurementInterval(uint16_t interval)
{
  return sendCommand(COMMAND_SET_MEASUREMENT_INTERVAL, interval);
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

  uint32_t tempCO2 = 0;
  uint32_t tempHumidity = 0;
  uint32_t tempTemperature = 0;

  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(COMMAND_READ_MEASUREMENT >> 8);   //MSB
  _i2cPort->write(COMMAND_READ_MEASUREMENT & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (0); //Sensor did not ACK

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
        tempCO2 <<= 8;
        tempCO2 |= incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      case 6:
      case 7:
      case 9:
      case 10:
        tempTemperature <<= 8;
        tempTemperature |= incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      case 12:
      case 13:
      case 15:
      case 16:
        tempHumidity <<= 8;
        tempHumidity |= incoming;
        bytesToCrc[x % 3] = incoming;
        break;
      default:
        //Validate CRC
        const uint8_t foundCrc = computeCRC8(bytesToCrc, 2);
        if (foundCrc != incoming)
        {
          //Serial.printf("Found CRC in byte %u, expected %u, got %u\n", x, incoming, foundCrc);
          error = true;
        }
        break;
      }
    }
  }
  else
  {
    //Serial.printf("No SCD30 data found from I2C, i2c claims we should receive %u bytes\n", receivedBytes);
    return false;
  }

  if (error)
  {
    //Serial.println("Encountered error reading SCD30 data.");
    return false;
  }
  //Now copy the uint32s into their associated floats
  memcpy(&co2, &tempCO2, sizeof(co2));
  memcpy(&temperature, &tempTemperature, sizeof(temperature));
  memcpy(&humidity, &tempHumidity, sizeof(humidity));

  //Mark our global variables as fresh
  co2HasBeenReported = false;
  humidityHasBeenReported = false;
  temperatureHasBeenReported = false;

  return (true); //Success! New data available in globals.
}

//Gets two bytes from SCD30
uint16_t SCD30::readRegister(uint16_t registerAddress)
{
  _i2cPort->beginTransmission(SCD30_ADDRESS);
  _i2cPort->write(registerAddress >> 8);   //MSB
  _i2cPort->write(registerAddress & 0xFF); //LSB
  if (_i2cPort->endTransmission() != 0)
    return (0); //Sensor did not ACK

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
