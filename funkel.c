#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIMER_INIT_VAL    (255 - 15)    // 1ms
#define RES_X             16
#define RES_Y             1
#define RES_Z             16

unsigned char led[RES_X];
unsigned long int rotation_time = 1;
unsigned long int time_count = 1;


void init_timer0() {
    TCCR0 = 0x00;       // Stop Timer/Counter0
    TCNT0 = TIMER_INIT_VAL;   // initial value
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0 = (1 << CS00) | (1 << CS01);        // 16.000.000/64
}

void init_interupt() {
	// disable alternate functions
	TCCR1A &= ~((1 << COM1B0) | (1 << COM1B1));

	GIMSK |= (1<<PCIE0);
	MCUCR |= (1<<ISC00) | (1<<ISC01);
	GIMSK |= (1<<INT0);      // turn on interrupts!

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
	if (time_count > 50) {
		rotation_time = (rotation_time + time_count) / 2;

		time_count = 1;
	}
}

void set_leds() {

	PORTA = (led[1] << PA0)
			| (led[3] << PA1)
			| (led[5] << PA2)
			| (led[7] << PA3)
	        | (led[9] << PA4)
	        | (led[11] << PA5)
	        | (led[13] << PA6)
	        | (led[15] << PA7);

	PORTB |= (1 << PB4);
	PORTB = 0xff ^ ((1 << PB4) | (1 << PB5));

	PORTA = (led[0] << PA0)
				| (led[2] << PA1)
				| (led[4] << PA2)
				| (led[6] << PA3)
		        | (led[8] << PA4)
		        | (led[10] << PA5)
		        | (led[12] << PA6)
		        | (led[14] << PA7);

	PORTB |= (1 << PB5);
	PORTB = 0xff ^ ((1 << PB4) | (1 << PB5));

}

void refresh (unsigned char current_z) {

	for (int i = 0; i < RES_X; i++) {
		if ((current_z < 8) == (i < 8)) {
			led[i] = 1;
		} else {
			led[i] = 0;
		}
	}

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
    	current_z = RES_Z * time_count / rotation_time;

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
