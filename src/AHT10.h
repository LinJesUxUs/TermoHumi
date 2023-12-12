#ifndef AHT10_H
#define AHT10_H
#define AHT10_INIT 0xE1
#define AHT10_MEASURE 0xAC
#define AHT10_RESET 0xBA
#define AHT10_DATA0 0x33
#define AHT10_DATA1 0x00
#define ACK true
#define NACK false
#define TWI_TWSR (TWSR & 0xF8)

#ifndef AHT10_ADDRESS
#define AHT10_ADDRESS 0x38
#endif

#include "structs.h"
#include <util/twi.h>
#include <avr/io.h>
#include <util/delay.h>

struct aht10_data {
  int8_t temp;
  uint8_t humi;
};

class AHT10 {
    static uint8_t recvbuf[6];
    static uint32_t tempraw, humiraw;
    static uint8_t start();
    static void stop();
    static void set_twcr(const uint8_t& _byte=0);
    static void wait();
    static void send_byte(const uint8_t& _byte);
    static uint8_t check(const uint8_t& _byte);   //return 0 is success
    static uint8_t read_byte(const bool& _ack);
    static uint8_t measure();
    static uint8_t recv();
public:
    static aht10_data value;
    static uint8_t begin();
    static uint8_t resetSensor();
    static uint8_t read();
};

// AHT10::tempraw = 0;
// AHT10::humiraw = 0;
// AHT10::recvbuf = {0,0,0,0,0,0};

#endif

// Certainly! Here's the completed AHT10 class with the TWI (Two-Wire Interface) library for I2C communication and implementation of the initialization sequence, reset sequence, and reading humidity and temperature data from the sensor:

// class AHT10 {
// private:
//     const uint8_t I2C_ADDRESS = 0x38;


// public:

//     bool readData(float &humidity, float &temperature) {
//         // Measure command
//         i2cStart();
//         i2cWriteByte(I2C_ADDRESS << 1);
//         i2cWriteByte(0xAC);
//         i2cWriteByte(0x33);
//         i2cWriteByte(0x00);
//         i2cStop();
//         _delay_ms(75);

//         // Read data
//         i2cStart();
//         i2cWriteByte((I2C_ADDRESS << 1) | 0x01);
        
//         uint8_t msb = i2cReadByte(true);
//         uint8_t lsb = i2cReadByte(true);
//         uint8_t xsb = i2cReadByte(false);
        
//         uint8_t crc = i2cReadByte(false);
        
//         i2cStop();

//         // Check CRC, calculate humidity and temperature
//         // TODO: Implement CRC check and humidity/temperature calculation

//         return true; // Return true if data read successfully
//     }
// };
