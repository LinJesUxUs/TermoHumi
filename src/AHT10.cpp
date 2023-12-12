#include "AHT10.h"

uint8_t AHT10::start(){
    set_twcr(1 << TWSTA);
    wait();
    return check(TW_START);
}

void AHT10::stop(){
    set_twcr(1 << TWSTO);
    _delay_ms(5); // Allow time for stop condition to complete
}

void AHT10::set_twcr(const uint8_t& _byte){
    TWCR = (1 << TWINT) | (1 << TWEN) | _byte;
}

void AHT10::wait(){
    while ( !( TWCR & (1<<TWINT) ) );
}

void AHT10::send_byte(const uint8_t& _byte){
    TWDR = _byte;
    set_twcr();  //send_int
    wait();
}

uint8_t AHT10::check(const uint8_t& _byte){
    if ((TWSR & 0xF8) != _byte)
        return _byte;
    return 0;
}

uint8_t AHT10::read_byte(const bool& _ack) {
    set_twcr(_ack << TWEA);
    wait();
    return TWDR;
}

uint8_t AHT10::measure(){
    if (start())
        return TWI_TWSR;
    send_byte(AHT10_ADDRESS << 1 | TW_WRITE);
    if (check(TW_MT_SLA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_MEASURE);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_DATA0);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_DATA1);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    stop();
    return 0;
}

uint8_t AHT10::recv(){
    if (start())
        return TWI_TWSR;
    send_byte(AHT10_ADDRESS << 1 | TW_READ);
    if (check(TW_MR_SLA_ACK))
        return TWI_TWSR;
    for (uint8_t i=0; i < 6; i++)
        recvbuf[i] = read_byte(ACK);
    return 0;
}

uint8_t AHT10::begin() {
    TWSR = 0;
    TWBR = ((F_CPU / 100000UL) - 16) / 8;
    TWCR = (1 << TWEN);
    // Init sequence
    if (start())
        return TWI_TWSR;
    send_byte((AHT10_ADDRESS << 1) | TW_WRITE);
    if (check(TW_MT_SLA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_INIT);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    send_byte(0x08);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    stop();
    _delay_ms(75);
    return 0;
}

uint8_t AHT10::resetSensor() {
    if (start())
        return TWI_TWSR;
    send_byte(AHT10_ADDRESS << 1 | TW_WRITE);
    if (check(TW_MT_SLA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_RESET);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    // i2cWriteByte(0xE1);
    // i2cWriteByte(0x08);
    stop();
    _delay_ms(20);
    return 0;
}

uint8_t AHT10::read(){
    if (measure())
        return TWI_TWSR;
    if (recv())
        return TWI_TWSR;
    tempraw = ( ( (uint32_t)recvbuf[3] & 15) << 16) | ( (uint32_t)recvbuf[4] << 8) | recvbuf[5];
    humiraw = ( (uint32_t)recvbuf[1] << 12) | ( (uint32_t)recvbuf[2] << 4) | ( recvbuf[3] >> 4);
    value.temp = ((tempraw/1048576)*200)-50;
    value.humi = (humiraw/1048576)*100;
    return 0;
}

// AHT10::AHT10(){
//     address = AHT10_ADDRESS;
//     value.humi = 0;
//     value.temp = 0;
//     TWBR = 0x02;
//     TWSR &= 0xFC;
//     start();
//     send_byte(address);
//     send_byte(0b11100001);
//     stop();
//     start();
//     send_byte(address);
//     send_byte(0b10101100);
//     stop();
// }