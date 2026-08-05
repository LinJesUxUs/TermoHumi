#include "AHT10.h"

aht10_data AHT10::value = {0,0};
uint32_t AHT10::tempraw = 0;
uint32_t AHT10::humiraw = 0;
uint8_t AHT10::recvbuf[] = {0,0,0,0,0,0};
uint8_t AHT10::check_buf = 0;

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

void AHT10::set_twcr(uint8_t _byte){
    TWCR = (1 << TWINT) | (1 << TWEN) | _byte;
}

void AHT10::wait(){
    while ( !( TWCR & (1<<TWINT) ) );
}

void AHT10::send_byte(uint8_t _data){
    TWDR = _data;
    set_twcr(0);  //send TWINT and TWEN
    wait();
}

uint8_t AHT10::check(uint8_t _expected){
    check_buf = TWSR & 0xF8;
    return (check_buf == _expected) ? 0 : check_buf;
}

uint8_t AHT10::send_and_check(uint8_t data, uint8_t expected_status) {
    send_byte(data);
    return check(expected_status);
}

uint8_t AHT10::read_byte(bool _ack) {
    set_twcr(_ack ? (1 << TWEA) : 0);
    wait();
    return TWDR;
}

uint8_t AHT10::measure(){
    if (start()) return TWI_TWSR;
 
    if (send_and_check( AHT10_ADDRESS << 1 | TW_WRITE, TW_MT_SLA_ACK) ) return TWI_TWSR;
    if (send_and_check( AHT10_MEASURE, TW_MT_DATA_ACK) ) return TWI_TWSR;
    if (send_and_check( AHT10_DATA0, TW_MT_DATA_ACK) ) return TWI_TWSR;
    if (send_and_check( AHT10_DATA1, TW_MT_DATA_ACK) ) return TWI_TWSR;
    
    stop();
    return 0;
}

uint8_t AHT10::recv(){
    if (start()) return TWI_TWSR;
    
    if (send_and_check( AHT10_ADDRESS << 1 | TW_READ, TW_MR_SLA_ACK) ) return TWI_TWSR;

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
    if (start()) return TWI_TWSR;

    if (send_and_check( (AHT10_ADDRESS << 1) | TW_WRITE, TW_MT_SLA_ACK)) return TWI_TWSR;
    if (send_and_check( AHT10_INIT, TW_MT_DATA_ACK)) return TWI_TWSR;

    if (send_and_check( 0x08, TW_MT_DATA_ACK)) return TWI_TWSR;    // first byte of initialization command
    if (send_and_check( 0x00, TW_MT_DATA_ACK)) return TWI_TWSR;    // second byte of initialization command
    
    stop();
    _delay_ms(75); // Wait for sensor to initialize
    return 0;
}

uint8_t AHT10::resetSensor() {
    if (start()) return TWI_TWSR;

    if (send_and_check( AHT10_ADDRESS << 1 | TW_WRITE, TW_MT_SLA_ACK)) return TWI_TWSR;
    if (send_and_check( AHT10_RESET, TW_MT_DATA_ACK)) return TWI_TWSR;
    
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
    tempraw = ( ( static_cast<uint32_t>(recvbuf[3]) & 15) << 16) | ( static_cast<uint32_t>(recvbuf[4]) << 8) | recvbuf[5];
    humiraw = ( static_cast<uint32_t>(recvbuf[1]) << 12) | ( static_cast<uint32_t>(recvbuf[2]) << 4) | ( recvbuf[3] >> 4);

    value.temp = static_cast<int16_t>( ( static_cast<int32_t>(tempraw) * 2000UL / 1048576UL ) - 500);
    value.humi = static_cast<int16_t>( static_cast<int32_t>(humiraw) * 1000UL / 1048576UL );
    return 0;
}

