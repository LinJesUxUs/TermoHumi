#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "AHT10.h"
#include "common.h"

#define BUTTON_ADC_CHANNEL 6  // 6 for ADC6, 7 for ADC7
#define BUTTON_THRESHOLD   85 // 1/3 from 255 (8-bit conversion)
#define BRIGHTNESS_LEVELS 4
#define EEPROM_SIZE 512

#define PERIOD_SEC 4
#define SEGMENT_NUM 4

// PORT B for programming
// PORT C for read AHT10 & select sector
// PORT D for flash leds "dFABGCDE" "3.10.11.7.5.4.2.1"


/* Initialise vars */

bool temp_or_humi = true;                // TRUE=temperature
uint8_t counter = 0;                     // Counter for multiplexing segments
uint8_t period_temphumi_counter = 0;     // Counter for switching between temperature and humidity display
volatile uint8_t current_brightness = 0; // Current brightness level (0-3). Reads from EEPROM on startup. Volatile to ensure it's updated in ISR context.
volatile uint8_t timer1_ticks = 0;      // Interrupt based clock.

// Structure to hold the segments for temperature and humidity
struct display_buf {
    uint8_t temp_segments[SEGMENT_NUM-1]; // From 4 segments, only 3 for digits
    uint8_t humi_segments[SEGMENT_NUM-1];
} tempHumi;

// Segments for characters H, C, and - on a 7-segment display (assuming common cathode)
uint8_t charSegments[3] = {
    0b01011101, 0b01100011, 0b00001000
};

// Brightness levels for PWM (0-3)
const uint8_t brightness_values[BRIGHTNESS_LEVELS] = {
    255, 170, 85, 20
};

// Segments for digits 0-9 on a 7-segment display (assuming common cathode)
uint8_t numberSegments[10] = {
    0b01110111, 0b00010100, 0b00111011, 0b00111110, 0b01011100,
    0b01101110, 0b01101111, 0b00110100, 0b01111111, 0b01111110
};


bool button_pressed() {
    ADCSRA |= (1 << ADSC);          // start conversion
    while (ADCSRA & (1 << ADSC));   // wait for completion
    return ADCH > BUTTON_THRESHOLD; // true if above 1/3 VCC
}

// Binary search for the last byte not equal to 0xFF
// Returns index or -1 if all are 0xFF
int16_t find_last_brightness() {
    int16_t left = 0;
    int16_t right = EEPROM_SIZE - 1;
    int16_t last = -1;
    
    while (left <= right) {
        int16_t mid = (left + right) / 2;
        uint8_t val = eeprom_read_byte( (uint8_t*)mid );
        
        if (val != 0xFF) {
            last = mid;         // candidate for the last record
            left = mid + 1;     // search to the right
        } else {
            right = mid - 1;    // empty, search to the left
        }
    }
    return last;
}

// Saves brightness to EEPROM, avoiding duplicates
void save_brightness(uint8_t level)
{
    int16_t last = find_last_brightness();

    if (last >= 0) {                                                        // Check if there is a valid record
        uint8_t last_val = eeprom_read_byte( (uint8_t*)last ); // Read the last valid record
        if (last_val == level)                                              // If the last record is the same as the new level, no need to write
            return;
    } else last = 0;                                                        // If no valid record, start at -1 so next becomes 0

    for (; last < EEPROM_SIZE; last++) {                                // Try to write to the next available cell
        eeprom_write_byte( (uint8_t*)last, level);         // Write the new brightness level
        if (eeprom_read_byte( (uint8_t*)last) == level)   // Verify the write
            break;                                                      // success
    }
}

void handle_brightness_button() {
    static bool last_state = false;
    static uint8_t debounce = 0;
    
    bool state = button_pressed();
    
    if (state != last_state) {
        debounce++;
        if (debounce > 10) {        // ~10 check cycles = ~50 ms
            if (state) {            // transition 0→1 = press
                current_brightness += 1;
                if ( current_brightness >= BRIGHTNESS_LEVELS)
                    current_brightness = 0;
                OCR2 = brightness_values[current_brightness];
                save_brightness(current_brightness);
            }
            debounce = 0;
            last_state = state;
        }
    } else {
        debounce = 0;
    }
}

// Loads brightness from EEPROM on startup
void load_brightness() {
    int16_t index = find_last_brightness();
    if (index >= 0) {
        uint8_t saved = eeprom_read_byte( (uint8_t*)index );
        if (saved < BRIGHTNESS_LEVELS) {
            current_brightness = saved;
        }
    }
    // otherwise current_brightness remains 0 (default)
    OCR2 = brightness_values[current_brightness];
}

void setDisplayDataArray(uint8_t* dataArray, int16_t value)
{
    bool point_on_tens = true;

    if (value < -99) {
        value /= 10;    // Limit to two digits for display
        point_on_tens = false;
    }

    if (value < 0) {
        dataArray[2] = charSegments[2]; // Set '-' for negative values
        value = -value;
    }
    else if (value >= 100)
        dataArray[2] = numberSegments[ static_cast<uint8_t>( (value / 100) % 10 ) ];    // Set hundreds digit for values >= 100
    else
        dataArray[2] = 0b00000000;                                                      // Clear the third segment for values < 100

    dataArray[1] = numberSegments[ static_cast<uint8_t>( (value / 10) % 10 ) ]          // Set tens digit
        | (point_on_tens ? 0b10000000 : 0b00000000);                                      // with/without decimal point

    dataArray[0] = numberSegments[ static_cast<uint8_t>( (value) % 10 ) ]               // Set units digit
        | (!point_on_tens ? 0b10000000 : 0b00000000);                                     // with/without decimal point
}

void getTempHumi()
{
    while (AHT10::read());

    int16_t t = AHT10::value.temp;
    int16_t h = AHT10::value.humi;
    
    setDisplayDataArray(tempHumi.temp_segments, t);
    setDisplayDataArray(tempHumi.humi_segments, h);
}

void init() {
    sei(); // Enable global interrupts
    
    // ADC initialization for button press detection
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128 (125 kHz at 16 MHz)
    ADMUX = (1 << ADLAR) | (BUTTON_ADC_CHANNEL & 0x07);                // Select channel, 8-bit conversion (ADLAR=1), reference AVCC
    
    load_brightness();  // restore brightness from EEPROM
    
    // Timer2 for PWM brightness control
    TCCR2 = (1 << WGM20) | (1 << WGM21) | (1 << COM21) | (1 << CS20); // Fast PWM, non-inverting mode, no prescaler. Frequency = F_CPU / 256 = 7.8125 kHz (for 2 MHz clock)
    DDRB |= (1 << PB3);                                               // Set PB3 (OC2) as output for PWM
    OCR2 = brightness_values[current_brightness];                     // Set initial brightness level

    // Timer1 for multiplexing display
    TCCR1B |= (1<<WGM12);             // CTC mode (clear timer on compare match)
    TIMSK |= (1<<OCIE1A);             // Enable Timer1 compare match interrupt
    OCR1AH = 0b00000000;              // Set OCR1A for 1 kHz interrupt frequency (assuming 2 MHz clock and prescaler of 64)
    OCR1AL = 0b01000001;              // 0x41 = 65 decimal, 2 MHz / (64 * 65) ≈ 480.77 Hz, adjust as needed
    TCCR1B |= (1<<CS11) | (1<<CS10) ; // Set prescaler to 64
    
    // AHT10 initialization
    while(AHT10::begin()) AHT10::resetSensor();
}

ISR (TIMER1_COMPA_vect) {
    timer1_ticks += 1;

    PORTC |= 0x0F;              // b00001111 Turn off all segments
    if (counter == SEGMENT_NUM-1)
        PORTD = charSegments[temp_or_humi]; // Display °C or % sign
    else {
        if (temp_or_humi)
            PORTD = tempHumi.temp_segments[counter];
        else
            PORTD = tempHumi.humi_segments[counter];
    }
    PORTC ^= 1 << counter;  // Turn on the current segment ( b00111111 ^ b00000100 = b00111011 )
    if (counter == SEGMENT_NUM-1 )
        counter = 0;
    else
        counter++;
}

int main() {
    init();
    for ( uint8_t i = 0; i < SEGMENT_NUM; i++ ){
        DDRC |= (1 << i);
    }
    DDRD = 0b11111111;

    int16_t last_tick = 0;
    const uint16_t ticks_per_500ms = 237; // Timer1: (65+1)*64/2MHz ≈ 2.112ms per tick => ~237 ticks per 500ms
    
    while (true) {
        handle_brightness_button();
        
        int16_t now = timer1_ticks;
        int16_t nowDiff = now - last_tick;
        if (nowDiff < 0) nowDiff += 256; // 
        if (nowDiff >= ticks_per_500ms) {
            last_tick = now;
            getTempHumi();
            period_temphumi_counter++;
            if ( period_temphumi_counter >= PERIOD_SEC ) {
                period_temphumi_counter = 0;
                temp_or_humi = !temp_or_humi;
            }
        }
    }
    return 0;
}

