SparkFun SCD30 CO₂ Sensor Library
===========================================================

![SparkFun SCD30 CO₂ Sensor](https://cdn.sparkfun.com//assets/parts/1/2/9/8/4/SparkFun_Sensirion_SCD30.jpg)

[*SparkX CO₂ Humidity and Temperature Sensor - SCD30 (SPX-14751)*](https://www.sparkfun.com/products/14751)

The SCD30 from Sensirion is a high quality [NDIR](https://en.wikipedia.org/wiki/Nondispersive_infrared_sensor) based CO₂ sensor capable of detecting 400 to 10000ppm with an accuracy of ±(30ppm+3%). In order to improve accuracy the SCD30 has temperature and humidity sensing built-in, as well as commands to set the current altitude.

We've written an Arduino library to make reading the CO₂, humidity, and temperature very easy. It can be downloaded through the Arduino Library manager: search for 'SparkFun SCD30'. We recommend using a [Qwiic Breadboard Cable](https://www.sparkfun.com/products/14425) to connect the SCD30 to a Qwiic compatible board. The Ye*LL*ow wire goes in the SC*L* pin. The SCD30 also supports a serial interface but we haven't worked with it.

The CO₂ sensor works very well and for additional accuracy the SCD30 accepts ambient pressure readings. We recommend using the SCD30 in conjunction with the [Qwiic Pressure Sensor - MS5637](https://www.sparkfun.com/products/14688) or the [Qwiic Environmental Sensor - BME680](https://www.sparkfun.com/products/14570) to obtain the current barometric pressure.

Note: The SCD30 has an automatic self-calibration routine. Sensirion recommends 7 days of continuous readings with at least 1 hour a day of 'fresh air' for self-calibration to complete.

Library written by Nathan Seidle ([SparkFun](http://www.sparkfun.com)).

Thanks to!

* [jobr97](https://github.com/jobr97) for adding the getTemperatureOffset() method
* [bobobo1618](https://github.com/bobobo1618) for writing a CRC check and improving the return values of the library
* [labeneator](https://github.com/labeneator) for adding method to disable calibrate at begin
* [AndreasExner](https://github.com/AndreasExner) for adding [reset and getAutoSelfCalibration methods](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/pull/17)
* [awatterott](https://github.com/awatterott) for adding [getAltitudeCompensation()](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/pull/18)
* [jogi-k](https://github.com/jogi-k) for adding [teensy i2clib](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/pull/19) support

Repository Contents
-------------------

* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **keywords.txt** - Keywords from this library that will be highlighted in the Arduino IDE. 
* **library.properties** - General library properties for the Arduino package manager. 

Documentation
--------------

* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.

License Information
-------------------

This product is _**open source**_! 

Various bits of the code have different licenses applied. Anything SparkFun wrote is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round! 

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release anything derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
