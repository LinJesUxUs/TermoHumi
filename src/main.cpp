#include "structs.h"
#include "AHT10.h"

// PORT B for programming
// PORT C for read AHT10 & select sector
// PORT D for flash leds "dFABGCDE" "3.10.11.7.5.4.2.1"

uint8_t numberSegments[10] = {
    0b01110111, 0b00010100, 0b00111011, 0b00111110, 0b01011100,
    0b01101110, 0b01101111, 0b00110100, 0b01111111, 0b01111110
};

uint8_t charSegments[3] = {
    0b01011101, 0b01100011, 0b00001000
};

display_buf tempHumi;

uint8_t counter = 0;
uint8_t period_temphumi_counter = 0;

bool temp_or_humi = true; // TRUE=temperature
bool temp_sign;

void getTempHumi(){
    while (AHT10::read());

    int8_t t = AHT10::value.temp;
    uint8_t h = AHT10::value.humi;
    
    if (t < 0 ) {
        temp_sign = true;
        t = -t;
    } else {
        temp_sign = false;
    }
    
    for (uint8_t i = 0; i < SEGMENT_NUM - 1; i++){
        tempHumi.temp_segments[i] = numberSegments[t % 10];
        tempHumi.humi_segments[i] = numberSegments[h % 10];
        t /= 10;
        h /= 10;
    }
    if(temp_sign)
        tempHumi.temp_segments[2] = charSegments[2];
    else if (tempHumi.temp_segments[2] == numberSegments[0])
	tempHumi.temp_segments[2] = 0b00000000;
    if (tempHumi.humi_segments[2] == numberSegments[0])
	tempHumi.humi_segments[2] = 0b00000000;
}

void timer_init(){
    TCCR1B |= (1<<WGM12);
    TIMSK |= (1<<OCIE1A);
    OCR1AH = 0b00000000;
    OCR1AL = 0b01000001;
    TCCR1B |= (1<<CS11) | (1<<CS10) ;
}

void aht10_init(){
    while(AHT10::begin()){
        AHT10::resetSensor();
	}
}

ISR (TIMER1_COMPA_vect) {
    PORTC |= 0x0F; // b00001111 Turn off all segments
    if (counter == SEGMENT_NUM-1)
        PORTD = charSegments[temp_or_humi]; // Display °C or % sign
    else {
        if (temp_or_humi)
            PORTD = tempHumi.temp_segments[counter];
        else
            PORTD = tempHumi.humi_segments[counter];
    }
    PORTC ^= 1 << counter; // Turn on the current segment ( b00111111 ^ b00000100 = b00111011 )
    if (counter == SEGMENT_NUM-1 )
        counter = 0;
    else
        counter++;
}

int main() {
    sei();
    timer_init();
    aht10_init();
    for ( uint8_t i = 0; i < SEGMENT_NUM; i++ ){
        DDRC |= (1 << i);
    }
    DDRD = 0b11111111;
    
    while(true){
        getTempHumi();
        if ( PERIOD_SEC <= period_temphumi_counter++){
            period_temphumi_counter = 0;
            temp_or_humi = !temp_or_humi;
        }
        _delay_ms(500);
    }
    return 0;
}
