#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIMER_INIT_VAL    255    // 1ms
#define RES_X             16
#define RES_Y             1
#define RES_Z             32

unsigned char led[RES_X/8];
unsigned long int rotation_time = 1;
unsigned long int time_count = 1;
unsigned char bild[RES_Z][RES_X/8] =
                          {
                        			{0b10000000, 0b00000001},
                        			{0b10000000, 0b10000000},
                        			{0b01000000, 0b10000000},
                        			{0b01000000, 0b01000000},
                        			{0b00100000, 0b01000000},
                        			{0b00100000, 0b00100000},
                        			{0b00010000, 0b00100000},
                        			{0b00010000, 0b00010000},
                        			{0b00001000, 0b00010000},
                        			{0b00001000, 0b00001000},
                        			{0b00000100, 0b00001000},
                        			{0b00000100, 0b00000100},
                        			{0b00000010, 0b00000100},
                        			{0b00000010, 0b00000010},
                        			{0b00000001, 0b00000010},
                        			{0b00000001, 0b00000001},
                        			{0b10000000, 0b00000001},
                        			{0b10000000, 0b10000000},
                        			{0b01000000, 0b10000000},
                        			{0b01000000, 0b01000000},
                        			{0b00100000, 0b01000000},
                        			{0b00100000, 0b00100000},
                        			{0b00010000, 0b00100000},
                        			{0b00010000, 0b00010000},
                        			{0b00001000, 0b00010000},
                        			{0b00001000, 0b00001000},
                        			{0b00000100, 0b00001000},
                        			{0b00000100, 0b00000100},
                        			{0b00000010, 0b00000100},
                        			{0b00000010, 0b00000010},
                        			{0b00000001, 0b00000010},
                        			{0b00000001, 0b00000001}
                          };


void init_timer0() {
    TCCR0 = 0x00;       // Stop Timer/Counter0
    TCNT0 = TIMER_INIT_VAL;   // initial value
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0 = (1 << CS02) | (1 << CS00);        // 8.000.000/1.024
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

unsigned long int get_current_time() {
	return time_count * TIMER_INIT_VAL + (TIMER_INIT_VAL - TCNT0);
}

/*
 * rising edge trigger
 */
ISR (IO_PINS_vect) {
	if (time_count > 200) {
		rotation_time = (rotation_time + get_current_time()) / 2;

		time_count = 0;
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

	led[0] = bild[current_z][0];
	led[1] = bild[current_z][1];

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
