#include <avr/io.h>
#include <avr/interrupt.h>

#include <avr/pgmspace.h>

#include "imgdata.h"

uint8_t volatile led[RES_Y/8];
uint32_t volatile rotation_time = 1;

uint32_t res_t = RES_T;

#define BANK_0 PB4
#define BANK_1 PB5
#define BANK_2 PB3
#define BANK_3 PB2

void init_timer0() {

    TCCR0B = 0x00;      // Stop Timer/Counter0
    TCNT0L = 0;         // initial value
    TCNT0H = 0;
    TCCR0A = _BV(TCW0);  // 16-bit Mode, Incrementing to 0xFFFF
    TCCR0B = (1 << CS02); // 8.000.000/256
    //TCCR0B = (1 << CS02) | (1 << CS00); // 8.000.000/1024

}

void init_interupt() {

    MCUCR |= (1<<ISC00) | (1<<ISC01); // rising edge
    GIMSK |= _BV(INT0);

}

uint32_t get_time() {

    uint32_t value;

    value = TCNT0H;
    value = value << 8;
    value |= TCNT0L;

    return value;

}

/*
 * rising edge trigger
 */
ISR (INT0_vect) {

    if (get_time() > (rotation_time >> 1)) {
        rotation_time = get_time();
        TCNT0H = 0;
        TCNT0L = 0;
    }

}

void apply_bank_change(int bank, int led_index) {

  // apply data from led array to PORTA pins
  PORTA = led[led_index];

  // switch (on) bank apply signal
  PORTB |= (1 << bank);

  // switch (off) bank apply signal
  PORTB ^= (1 << bank);

}

void set_leds() {
/*
    led[0] = 0;
    led[1] = 0;
    led[2] = 0;
    led[3] = 0;
*/
  apply_bank_change(BANK_0, 0);
  apply_bank_change(BANK_1, 1);
  apply_bank_change(BANK_2, 2);
  apply_bank_change(BANK_3, 3);

}

void display_image_data (uint32_t time_index) {

    // read data from program memory into led arrays
    led[0] = pgm_read_byte(&(imgdata[time_index][3]));
    led[1] = pgm_read_byte(&(imgdata[time_index][2]));
    led[2] = pgm_read_byte(&(imgdata[time_index][1]));
    led[3] = pgm_read_byte(&(imgdata[time_index][0]));

    set_leds();

}


void display_standby (uint8_t val) {

    led[3] = val;
    led[2] = val;
    led[1] = val;
    led[0] = val;
    
    set_leds();

}


void display_lauflicht (uint32_t time_index) {
    
    time_index /= (res_t / 32);
    
    if(time_index < 8) {
        led[0] = (1 << time_index);
    } else if (time_index < 16) {
        led[1] = (1 << (time_index - 8));
    } else if (time_index < 24) {
        led[2] = (1 << (time_index - 16));
    } else if (time_index < 32) {
        led[3] = (1 << (time_index - 24));
    }

    set_leds();

}


void display_time_index (uint32_t time_index) {
    
    uint32_t val = time_index;

    led[3] = 0;
    led[2] = val >> 16;
    led[1] = val >> 8;
    led[0] = val;
    
    set_leds();

}

int main() {

    init_interupt();
    init_timer0();

    DDRA  = 0xFF;
    PORTA = 0x00;
    DDRB  = (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5);
    PORTB = 0x00;

    led[0] = 0;
    led[1] = 0;
    led[2] = 0;
    led[3] = 0;
    set_leds();
    
    sei();

    uint32_t last_t = res_t + 1;
    uint32_t time_index;

    while (1) {

        //    time_indeximer_ticks      x                time_indeximer_ticks * RES_T
        //   --------------------- = -----    =>    x = ---------------------------
        //   average_rotation_time   RES_T                 average_rotation_time
        time_index = res_t * get_time() / rotation_time;
        
        if (time_index != last_t) {
            last_t = time_index;
            
            if (get_time() > rotation_time) {
                display_standby(0xF0);
            }
            else if(time_index >= res_t) {
                display_standby(0b11001100);
            } else {
                display_image_data(time_index);
            }
        }

    }

    return 0;

}
