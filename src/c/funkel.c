#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "imgdata.h"

unsigned char led[RES_Y/8];
unsigned long int rotation_time = 1;
unsigned long int time_count = 1;

#define RTMAX 5
unsigned char rtcurrent = 0;
unsigned long int rtcollector[RTMAX];

#define BANK_0 PB4
#define BANK_1 PB5
#define BANK_2 PB3
#define BANK_3 PB2

void init_timer0() {

    TCCR0B = 0x00; // Stop Timer/Counter0
    TCNT0L = 0; // initial value
    TCCR0A = 0; // 8-bit Mode, Incrementing to 0xFF
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0B = (1 << CS01) | (1 << CS00); // 8.000.000/64

}

void init_interupt() {

    MCUCR |= (1<<ISC00) | (1<<ISC01); // rising edge

    GIMSK |= (1<<INT0); // turn on interrupt INT0

}

unsigned long int get_current_time() {

   return time_count * 0xFF + TCNT0L;

}

/*
 * timer
 */
ISR(TIMER0_OVF_vect) {

    time_count++;

    if (time_count > 50) {
    	time_count = 0;
    }

}

/*
 * rising edge trigger
 */
ISR (INT0_vect) {

	// damit nich an einem vorbeilaufen an der ir-led mehrmals getriggert wird
    if (TCNT0L > 10 || time_count > 0) {
    	++rtcurrent;
    	if (rtcurrent == RTMAX) rtcurrent = 0;
    	rtcollector[rtcurrent] = get_current_time();

    	unsigned long int rtsum = 0;
    	for (int i=0; i<RTMAX; i++) {
    		rtsum += rtcollector[i];
    	}

        rotation_time = rtsum / RTMAX;

        time_count = 0;
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

	apply_bank_change(BANK_0, 0);
	apply_bank_change(BANK_1, 1);
	apply_bank_change(BANK_2, 2);
	apply_bank_change(BANK_3, 3);

}

void refresh (unsigned int current_t) {

    led[0] = pgm_read_byte(&(imgdata[current_t][3]));
    led[1] = pgm_read_byte(&(imgdata[current_t][2]));
    led[2] = pgm_read_byte(&(imgdata[current_t][1]));
    led[3] = pgm_read_byte(&(imgdata[current_t][0]));

/*
	led[0] = 0;
	led[1] = 0;
	led[2] = 0;
	led[3] = 0;


	if(current_t < 8) {
		led[0] = (1 << current_t);
	} else if (current_t < 16) {
		led[1] = (1 << (current_t - 8));
	} else if (current_t < 24) {
		led[2] = (1 << (current_t - 16));
	} else if (current_t < 32) {
		led[3] = (1 << (current_t - 24));
	}
*/
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

    unsigned int last_t = RES_T + 1;
    unsigned int current_t;

    rotation_time = 0xFF * 16;

    while (1) {

	current_t = (RES_T * get_current_time() / rotation_time) % RES_T;

    	if (current_t == last_t) {
    	    asm("sleep"::);
    	}
    	else {
    	    last_t = current_t;
    	    refresh(current_t);
    	}

    }

    return 0;

}
