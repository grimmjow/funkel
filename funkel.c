#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIMER_INIT_VAL    (255 - 15)    // 1ms
#define X_RES             64

unsigned char led[8];
unsigned long int rotation_time = 1;
unsigned long int time_count = 1;


void init_timer0() {
    TCCR0 = 0x00;       // Stop Timer/Counter0
    TCNT0 = TIMER_INIT_VAL;   // initial value
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0 = (1 << CS00) | (1 << CS02);        // 16.000.000/1.024
}

void init_interupt() {
	// disable alternate functions
	TCCR1A &= ~((1 << COM1B0) | (1 << COM1B1));

	GIMSK |=  (1<<PCIE0);
	GIMSK |= (1<<INT0);      // turn on interrupts!
	MCUCR |= (1<<ISC00) | (1<<ISC01);

}

/*
 * timer
 */
ISR(TIMER0_OVF0_vect) {
    time_count++;

    TCNT0 = TIMER_INIT_VAL;  // Clear Time/Counter0
}

/*
 * rising edge trigger
 */
ISR (IO_PINS_vect) {
	rotation_time = time_count;

	time_count = 1;
}

void set_leds() {

}

void refresh (unsigned char current_x) {

}

int main() {
    init_interupt();
    init_timer0();

    DDRA  = _BV(PA0);
    PORTA |= _BV(PA0);
    PORTB = 0x00;
    DDRB  = 0;

    sei();

    unsigned char last_x = X_RES + 1;
    unsigned char current_x;

    while (1) {
    	current_x = X_RES / (rotation_time / time_count);

    	if (current_x == last_x) {
    		asm("sleep"::);
    	}
    	else {
    		refresh(current_x);
    	}
    }

    return 0;
}
