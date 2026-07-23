#include "AHT10.h"

aht10_data AHT10::value = {0,0};
uint32_t AHT10::tempraw = 0;
uint32_t AHT10::humiraw = 0;
uint8_t AHT10::recvbuf[] = {0,0,0,0,0,0};
uint8_t AHT10::check_buf = 0;
bool AHT10::onecheck = false;

uint8_t AHT10::start(){
    set_twcr(1 << TWSTA);
    wait();
    check_buf = TWSR & 0xF8;
    if (check_buf == TW_START || check_buf == TW_REP_START)
	return 0;
    return check_buf;
}

void AHT10::stop(){
    set_twcr(1 << TWSTO);
    _delay_us(10); // Allow time for stop condition to complete
}

void AHT10::set_twcr(const uint8_t& _byte){
    TWCR = (1 << TWINT) | (1 << TWEN) | _byte;
}

void AHT10::wait(){
    while ( !( TWCR & (1<<TWINT) ) );
}

void AHT10::send_byte(const uint8_t& _data){
    TWDR = _data;
    set_twcr(0);  //send TWINT and TWEN
    wait();
}

uint8_t AHT10::check(const uint8_t& _expected){
    check_buf = TWSR & 0xF8;
    return (check_buf == _expected) ? 0 : _expected;
}

uint8_t AHT10::read_byte(const bool& _ack) {
    set_twcr(_ack ? (1 << TWEA) : 0);
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
    for (uint8_t i=0; i < 5; i++)
        recvbuf[i] = read_byte(ACK);
    recvbuf[5] = read_byte(NACK);
    stop();
    return 0;
}

uint8_t AHT10::begin() {
    TWSR = 0;
    TWBR = ((F_CPU / 100000UL) - 16) / 8;
    TWCR = (1 << TWEN);

    _delay_ms(40); // Wait for sensor to power up

    // Init sequence
    if (start())
        return TWI_TWSR;
    send_byte((AHT10_ADDRESS << 1) | TW_WRITE);
    if (check(TW_MT_SLA_ACK))
        return TWI_TWSR;
    send_byte(AHT10_INIT);
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    send_byte(0x08); // first byte of initialization command
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    send_byte(0x00); // second byte of initialization command
    if (check(TW_MT_DATA_ACK))
        return TWI_TWSR;
    stop();
    _delay_ms(75); // Wait for sensor to initialize
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
    stop();
    _delay_ms(20);
    return 0;
}

uint8_t AHT10::read(){
    if (measure())
        return TWI_TWSR;
    _delay_ms(80);
    if (recv())
        return TWI_TWSR;
    tempraw = ( ( (uint32_t)recvbuf[3] & 15) << 16) | ( (uint32_t)recvbuf[4] << 8) | recvbuf[5];
    humiraw = ( (uint32_t)recvbuf[1] << 12) | ( (uint32_t)recvbuf[2] << 4) | ( recvbuf[3] >> 4);
    value.temp = (int8_t)( ( (int32_t)tempraw * 200UL / 1048576UL ) - 50);
    value.humi = (uint8_t)( (uint32_t)humiraw * 100UL / 1048576UL );
    return 0;
}

void AHT10::set_value(const int8_t& _t, const uint8_t& _h) {
    value.humi=_h;
    value.temp=_t;
}
