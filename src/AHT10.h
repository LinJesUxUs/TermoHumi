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

#include "common.h"
#include <util/twi.h>
#include <avr/io.h>
#include <util/delay.h>

struct aht10_data {
    int16_t temp;
    int16_t humi;
};

class AHT10 {
    static uint8_t recvbuf[6];
    static uint32_t tempraw, humiraw;
    static uint8_t check_buf;
    static uint8_t start();
    static void stop();
    static void set_twcr(uint8_t _byte=0); // sets _byte in TWI Control Register. TWINT and TWEN inlined
    static void wait();
    static void send_byte(uint8_t _byte);
    static uint8_t check(uint8_t _byte);   // return 0 is success
    static uint8_t send_and_check(uint8_t data, uint8_t expected_status);
    static uint8_t read_byte(bool _ack);
    static uint8_t measure();
    static uint8_t recv();
public:
    static aht10_data value;
    static uint8_t begin();
    static uint8_t resetSensor();
    static uint8_t read();
};

#endif
