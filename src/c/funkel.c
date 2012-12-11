#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "imgdata.h"

unsigned char led[RES_X/8];
unsigned long int rotation_time = 1;
unsigned long int time_count = 1;


void init_timer0() {

    TCCR0B = 0x00; // Stop Timer/Counter0
    TCNT0L = 0; // initial value
    TCCR0A = 0; // 8-bit Mode, Incrementing to 0xFF
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0B = (1 << CS02) | (1 << CS00); // 8.000.000/1.024

}

void init_interupt() {
    // disable alternate functions
    TCCR1A &= ~((1 << COM1B0) | (1 << COM1B1));

    MCUCR |= (1<<ISC00) | (1<<ISC01); // rising edge
    GIMSK |= (1<<INT0); // turn on interrupts!

}

unsigned long int get_current_time() {

   return time_count * 0xFF + TCNT0L;

}

/*
 * timer
 */
ISR(TIMER0_OVF_vect) {

    time_count++;

}

/*
 * rising edge trigger
 */
ISR (INT0_vect) {

	// damit nich an einem vorbeilaufen an der ir-led mehrmals getriggert wird
    if (TCNT0L > 10 || time_count > 0) {
        rotation_time = (rotation_time + get_current_time()) / 2;

        time_count = 0;
        TCNT0L = 0;
    }

}

void set_leds() {

    PORTA = led[1];

    PORTB |= (1 << PB4);
    PORTB = 0xff ^ ((1 << PB4) | (1 << PB5));

    PORTA = led[0];

    PORTB |= (1 << PB5);
    PORTB = 0xff ^ ((1 << PB4) | (1 << PB5));

}

void refresh (unsigned char current_z) {

    led[0] = pgm_read_byte(&(imgdata[current_z][1]));
    led[1] = pgm_read_byte(&(imgdata[current_z][0]));

    set_leds();

}

int main() {
    init_interupt();
    init_timer0();

    DDRA  = 0xFF;
    PORTA = 0xFF;
    DDRB  = (1 << PB4) | (1 << PB5);
    PORTB = 0xFF;

    sei();

    unsigned long last_z = RES_Z + 1;
    unsigned long current_z;

    while (1) {
        current_z = RES_Z * get_current_time() / rotation_time;

        if (current_z == last_z) {
            asm("sleep"::);
        }
        else {
            last_z = current_z;
            refresh(current_z);
        }
    }

    return 0;
}
