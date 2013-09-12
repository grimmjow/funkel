#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "imgdata.h"

unsigned char led[RES_Y/8];
unsigned long int rotation_time = 1;
unsigned long int timer_interrupt_count = 1;

#define ROTATION_TIME_COLLECT_MAX 5
unsigned char rotation_collect_count = 0;
unsigned long int rotation_time_collector[ROTATION_TIME_COLLECT_MAX];

#define BANK_0 PB4
#define BANK_1 PB5
#define BANK_2 PB3
#define BANK_3 PB2

#define TIMER_INTERRUPT_OVERFLOW_VALUE 0xff

void init_timer0() {

    TCCR0B = 0x00;      // Stop Timer/Counter0
    TCNT0L = 0;         // initial value
    TCCR0A = 0;         // 8-bit Mode, Incrementing to 0xFF
    TIMSK = _BV(TOIE0); // Overflow Interrupt Enable
    TCCR0B = (1 << CS01) | (1 << CS00); // 8.000.000/64

}

void init_interupt() {

    MCUCR |= (1<<ISC00) | (1<<ISC01); // rising edge

    GIMSK |= (1<<INT0); // turn on interrupt INT0

}

unsigned long int get_time_indexime() {
  
  // (timer interrupts) * (timer counts per timer interrupt) + (current timer count)
   return timer_interrupt_count * TIMER_INTERRUPT_OVERFLOW_VALUE + TCNT0L;

}

/*
 * timer
 */
ISR(TIMER0_OVF_vect) {

    timer_interrupt_count++;

    // TODO: es kann sein,das das hier bullshit ist.
    // wenn der prescaler bei 8.000.000/64 steht, bekommen wir
    // 125.000 mal in der sekunde einen interrupt. D.h. timer_interrupt_counter
    // wird 2.500 mal pro sekunde auf 0 zurück gesetzt (125.000 / 50). Im Gegenzug 
    // bekommen wir bei 5 rotationen/s, 25.0000 timer interrupts pro rotation (also 500 
    // mal reset auf 0, pro sekunde).
    // Soll heissen, wir verpassen 495 * 255 timer counts. Warum ist das so?
    //if (timer_interrupt_count > 50) {
    //	timer_interrupt_count = 0;
    //}

}

/*
 * rising edge trigger
 */
ISR (INT0_vect) {

    // damit nich an einem vorbeilaufen an der ir-led mehrmals getriggert wird
    if (TCNT0L > 10 || timer_interrupt_count > 0) {
      
        // collect rotation timings
        if (rotation_collect_count == ROTATION_TIME_COLLECT_MAX) {
                rotation_collect_count = 0;
        }
        rotation_time_collector[rotation_collect_count] = get_time_indexime();
        rotation_collect_count++;

        // calculate average rotation timing
        unsigned long int rotation_time_sum = 0;
        for (int i = 0; i < ROTATION_TIME_COLLECT_MAX; i++) {
            rotation_time_sum += rotation_time_collector[i];
        }

        rotation_time = rotation_time_sum / ROTATION_TIME_COLLECT_MAX;

        // reset timer components
        timer_interrupt_count = 0;
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

void display_image_data (unsigned int time_index) {

    // read data from program memory into led arrays
    led[0] = pgm_read_byte(&(imgdata[time_index][3]));
    led[1] = pgm_read_byte(&(imgdata[time_index][2]));
    led[2] = pgm_read_byte(&(imgdata[time_index][1]));
    led[3] = pgm_read_byte(&(imgdata[time_index][0]));

/*
    led[0] = 0;
    led[1] = 0;
    led[2] = 0;
    led[3] = 0;

    
    if(time_index < RES_T/4) {
        led[0] = (1 << time_index);
    } else if (time_index < (RES_T/4)*2) {
        led[1] = (1 << (time_index - RES_T/4));
    } else if (time_index < (RES_T/4)*3) {
        led[2] = (1 << (time_index - (RES_T/4)*2));
    } else if (time_index < RES_T) {
        led[3] = (1 << (time_index - (RES_T/4)*3));
    }
*/
    set_leds();

}


void display_standby () {

    led[0] = 0b00110011;
    led[1] = led[0];
    led[2] = led[0];
    led[3] = led[0];
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
    unsigned int time_index;

    // default average rotation time: 16 interrupts * 255 (counts per interrupt) = 4.080 timer counts
    rotation_time = TIMER_INTERRUPT_OVERFLOW_VALUE * 16;

    while (1) {

        //    time_indeximer_ticks      x                time_indeximer_ticks * RES_T
        //   --------------------- = -----    =>    x = ---------------------------
        //   average_rotation_time   RES_T                 average_rotation_time
        time_index = RES_T * get_time_indexime() / rotation_time;

        if (time_index == last_t) {
            // do nothing if time index hasn't changed
            asm("sleep"::);
        }
        else {
            last_t = time_index;
            if(time_index > RES_T) {
                display_standby();
            } else {
                display_image_data(time_index);
            }
        }

    }

    return 0;

}
