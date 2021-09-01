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

#ifndef __SparkFun_SCD30_ARDUINO_LIBARARY_H__
#define __SparkFun_SCD30_ARDUINO_LIBARARY_H__

// Uncomment the next #define if using an Teensy >= 3 or Teensy LC and want to use the dedicated I2C-Library for it
// Then you also have to include <i2c_t3.h> on your application instead of <Wire.h>

// #define USE_TEENSY3_I2C_LIB

#include "Arduino.h"
#ifdef USE_TEENSY3_I2C_LIB
#include <i2c_t3.h>
#else
#include <Wire.h>
#endif

//The default I2C address for the SCD30 is 0x61.
#define SCD30_ADDRESS 0x61

//Available commands

#define COMMAND_CONTINUOUS_MEASUREMENT 0x0010
#define COMMAND_SET_MEASUREMENT_INTERVAL 0x4600
#define COMMAND_GET_DATA_READY 0x0202
#define COMMAND_READ_MEASUREMENT 0x0300
#define COMMAND_AUTOMATIC_SELF_CALIBRATION 0x5306
#define COMMAND_SET_FORCED_RECALIBRATION_FACTOR 0x5204
#define COMMAND_SET_TEMPERATURE_OFFSET 0x5403
#define COMMAND_SET_ALTITUDE_COMPENSATION 0x5102
#define COMMAND_RESET 0xD304 // Soft reset
#define COMMAND_STOP_MEAS 0x0104
#define COMMAND_READ_FW_VER 0xD100

typedef union
{
	byte array[4];
	float value;
} ByteToFl; // paulvha

class SCD30
{
public:
	SCD30(void);

	bool begin(bool autoCalibrate) { return begin(Wire, autoCalibrate); }
#ifdef USE_TEENSY3_I2C_LIB
	bool begin(i2c_t3 &wirePort = Wire, bool autoCalibrate = false, bool measBegin = true); //By default use Wire port
#else
	bool begin(TwoWire &wirePort = Wire, bool autoCalibrate = false, bool measBegin = true); //By default use Wire port
#endif

	void enableDebugging(Stream &debugPort = Serial); //Turn on debug printing. If user doesn't specify then Serial will be used.

	bool beginMeasuring(uint16_t pressureOffset);
	bool beginMeasuring(void);
	bool StopMeasurement(void); // paulvha

	bool setAmbientPressure(uint16_t pressure_mbar);

	bool getSettingValue(uint16_t registerAddress, uint16_t *val);
	bool getFirmwareVersion(uint16_t *val) { return (getSettingValue(COMMAND_READ_FW_VER, val)); }
	uint16_t getCO2(void);
	float getHumidity(void);
	float getTemperature(void);

	uint16_t getMeasurementInterval(void);
	bool getMeasurementInterval(uint16_t *val) { return (getSettingValue(COMMAND_SET_MEASUREMENT_INTERVAL, val)); }
	bool setMeasurementInterval(uint16_t interval);

	uint16_t getAltitudeCompensation(void);
	bool getAltitudeCompensation(uint16_t *val) { return (getSettingValue(COMMAND_SET_ALTITUDE_COMPENSATION, val)); }
	bool setAltitudeCompensation(uint16_t altitude);

	bool getAutoSelfCalibration(void);
	bool setAutoSelfCalibration(bool enable);

	bool getForcedRecalibration(uint16_t *val) { return (getSettingValue(COMMAND_SET_FORCED_RECALIBRATION_FACTOR, val)); }
	bool setForcedRecalibrationFactor(uint16_t concentration);

	float getTemperatureOffset(void);
	bool getTemperatureOffset(uint16_t *val) { return (getSettingValue(COMMAND_SET_TEMPERATURE_OFFSET, val)); }
	bool setTemperatureOffset(float tempOffset);

	bool dataAvailable();
	bool readMeasurement();

	void reset();

	bool sendCommand(uint16_t command, uint16_t arguments);
	bool sendCommand(uint16_t command);

	uint16_t readRegister(uint16_t registerAddress);

	uint8_t computeCRC8(uint8_t data[], uint8_t len);

private:
	//Variables
#ifdef USE_TEENSY3_I2C_LIB
	i2c_t3 *_i2cPort; //The generic connection to user's chosen I2C hardware
#else
	TwoWire *_i2cPort;																		 //The generic connection to user's chosen I2C hardware
#endif
	//Global main datums
	float co2 = 0;
	float temperature = 0;
	float humidity = 0;

	//These track the staleness of the current data
	//This allows us to avoid calling readMeasurement() every time individual datums are requested
	bool co2HasBeenReported = true;
	bool humidityHasBeenReported = true;
	bool temperatureHasBeenReported = true;

	//Debug
	Stream *_debugPort;			 //The stream to send debug messages to if enabled. Usually Serial.
	boolean _printDebug = false; //Flag to print debugging variables
};
#endif
