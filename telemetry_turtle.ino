/* 
Thanks to (c) Michael Schoeffler 2017, http://www.mschoeffler.de
and rydepier https://rydepier.wordpress.com (based on code by Tom Igoe
// 
This example shows how to write accelerometer/gyro/temp data to an SD card
using the SD library
//
The circuit: 
* SD card attached to the SPI bus as follows
** MOSI - pin D7
** MISO - pin 
** CLK - pin
** CS - pin D4
*
*/


// include the SD library:
#include <SPI.h>
#include <SD.h>
// This library allows you to communicate with I2C devices.
#include "Wire.h" 

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
const int chipSelect = 8; // The Sparkfun microSD shield uses pin 8 for CS

int16_t accel_x, accel_y, accel_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temp; // variables for temperature data

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

String dataString = ""; // holds the data to be written to the SD
File sensorData;

void setup() {
  Serial.begin(9600);
  Serial.print("Initializing SD card...");

  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(10, OUTPUT);
  // The chipSelect pin you use should also be set to output
  pinMode(chipSelect, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accel_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accel_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accel_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temp = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
 
  dataString = String(accel_x)+ "," + String(accel_y) + "," + String(accel_z) + "," +
               String(temp/340.00+36.53) + "," + String(gyro_x) + "," + String(gyro_y) + "," + String(gyro_z);

  saveData(); // save to SD card
  
  // delay as required
  delay(100);
}

void saveData() {
  sensorData = SD.open("datalog.csv", FILE_WRITE);
  if (sensorData){
    sensorData.println(dataString);
    sensorData.close(); // close the file
    // print to the serial port too: 
    Serial.println(dataString);
  }
  else{
    Serial.println("Error writing to file !");
  }
}
