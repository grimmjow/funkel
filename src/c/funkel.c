#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "imgdata.h"

uint8_t led[RES_Y/8];
uint8_t volatile int0_triggered = 0;
uint32_t res_t = RES_T;
uint32_t time_index;
uint32_t rotation_time = 0x0000FFFF;
/*
uint8_t debug_led=1;
*/
#define BANK_0 PB4
#define BANK_1 PB5
#define BANK_2 PB3
#define BANK_3 PB2

void init_timer0() {

    TCCR0B |= _BV(PSR0);      // Stop Timer/Counter0
    TCNT0H = 0;
    TCNT0L = 0;         // initial value
    TCCR0A |= _BV(TCW0);  // 16-bit Mode, Incrementing to 0xFFFF
    //TCCR0B |= _BV(CS00); // no prescale
    //TCCR0B |= _BV(CS01); // 8.000.000/8
    //TCCR0B |= _BV(CS02); // 8.000.000/64
    TCCR0B |= _BV(CS02); // 8.000.000/256
    //TCCR0B |= (1 << CS02) | (1 << CS00); // 8.000.000/1024

}

void init_interupt() {

    //MCUCR |= _BV(ISC01); // any change
    MCUCR |= _BV(ISC00) | _BV(ISC01); // rising edge
    GIMSK = _BV(INT0);
    PCMSK0 = 0;
    PCMSK1 = 0;

}

uint32_t get_time() {

    uint32_t value;

    value = TCNT0H;
    value = value << 8;
    value |= TCNT0L;

    return value;

}

void apply_bank_change(int bank, int led_index) {

  // apply data from led array to PORTA pins
  PORTA = led[led_index];

  // switch (on) bank apply signal
  PORTB |= _BV(bank);

  // switch (off) bank apply signal
  PORTB &= ~_BV(bank);
  
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


void display_image_data () {
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

void display_lauflicht () {
    
    time_index /= (res_t / 32);
	
    led[0] = 0;
    led[1] = 0;
    led[2] = 0;
    led[3] = 0;
    
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
    
    uint8_t val = 0xFF & time_index;

    led[3] = int0_triggered;
    led[2] = 0;
    led[1] = 0;
    led[0] = val;
    
    set_leds();

}

void set_leds_off() {

    display_standby(0x00);
    uint8_t val = 1;

    for(int i = 0; i < 16; i++) {
        
        display_standby(val);
        _delay_ms(5);
        
        if(i < 7) { 
            val = val << 1;
        } else {
            val = val >> 1;
        }
        
    }

}
/*
 * rising edge trigger
 */

ISR (INT0_vect) {

    if (TCNT0H > 0x01) {
        // check if PINB6 is low
        //if(! (PINB & (1<<PINB6))) {
            int0_triggered = 1;
        //}
    }
    
}


int main() {
    
    wdt_disable();

    init_interupt();
    init_timer0();

    DDRA  = 0xFF;
    PORTA = 0x00;

    DDRB  = (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5);
    // enable pull-up resistors
    PORTB |= _BV(PB0) | _BV(PB1) | _BV(PB6) | _BV(PB7);

    set_leds_off();

    uint32_t last_t = res_t + 1;
    uint32_t time;

    sei();

    while (1) {
    
        /*cli();
        display_time_index(get_time());
        sei();*/
        //    time_indeximer_ticks      x                time_indeximer_ticks * RES_T
        //   --------------------- = -----    =>    x = ---------------------------
        //   average_rotation_time   RES_T                 average_rotation_time

        if(int0_triggered) {
            rotation_time = get_time();
            TCNT0H = 0;
            TCNT0L = 0;
            int0_triggered = 0;
        }       
        
        time = get_time();
        /*
        if(time > rotation_time) {

            time = time % rotation_time;
            
        };
        */
        time_index = res_t * time / rotation_time;

        if (time_index != last_t) {
            
            last_t = time_index;

            if (time > rotation_time) {
                //display_standby(0b01010101);
            } else {
                display_image_data();
                //display_lauflicht();
                //display_time_index(time_index);
            }
            
        }

        //display_standby(0xFF);
        /*
        if(int0_triggered) {
             display_standby(0xFF);
             int0_triggered = 0;
        } else {
             display_standby(0x00);
             int0_triggered = 1;
        }
        
        _delay_ms(10);
        */
        
    }

    return 0;

}
