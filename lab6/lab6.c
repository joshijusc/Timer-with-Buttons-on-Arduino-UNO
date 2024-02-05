/********************************************
 *
 *  Name: Jason Joshi
 *  Email: joshij@usc.edu
 *  Section: Friday 11am
 *  Assignment: Lab 6 - Timers
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"

void debounce(uint8_t);

void timer1_init(void);
volatile unsigned ones = 0;
volatile unsigned tens = 0;
volatile unsigned tenths = 0;

enum states { PAUSE, RUN, LAP, STARTRUN};
uint8_t state = PAUSE;

int main(void) {



    // Initialize the LCD and TIMER1
    lcd_init();
    timer1_init();


    // Enable pull-ups for buttons
    PORTC |= (1 << PC2) | (1 << PC4);


    // Show the splash screen
    char buf[12];
    char* title = "EE109 Lab 6";
    snprintf(buf, 12, "%s", title);
    lcd_moveto(0, 0);
    lcd_stringout(buf);
    char* name = "Jason Joshi";
    snprintf(buf, 12, "%s", name);
    lcd_moveto(1, 0);
    lcd_stringout(buf);
    _delay_ms(1000);
    lcd_writecommand(1);

    // Enable interrupts
    sei();

    DDRC |= (1 << PC5);
    TCCR1B |= (1 << CS11) | (1 << CS10);
    state = PAUSE;
    lcd_moveto(0, 0);
    char temp[15];
    snprintf(temp, 15, "%d%d.%d", tens, ones, tenths);
    lcd_stringout(temp);
    char input;

    while (1) {                 // Loop forever
        // Read the buttons
        input = PINC;
        if (state == PAUSE) {               // PAUSE state
            if ((input & (1<<PC2)) == 0) {
                _delay_ms(5);
                state = STARTRUN;
            }
            if ((input & (1<<PC4)) == 0) {
                debounce(PC4);
                //reset to 0;
                tens = 0;
                ones = 0;
                tenths = 0;
                lcd_moveto(0, 0);
                snprintf(temp, 15, "%d%d.%d", 0, 0, 0);
                lcd_stringout(temp);
            }
        }
        else if (state == STARTRUN){
            if ((input & (1<<PC2)) != 0){
                _delay_ms(5);
                state = RUN;
            }
        }
        else if (state == RUN) {            // RUN state
            if ((input & (1<<PC4)) == 0) {
                debounce(PC4);
                state = LAP;
                // show same time on screen but keep timer going
            }
            if ((input & (1<<PC2)) == 0) {
                debounce(PC2);
                state = PAUSE;
                // pause time
            }
        }
        else if (state == LAP) {            // LAP state
            if (((input & (1<<PC2)) == 0)){
                debounce(PC2);
                state = RUN;
                //show current time running
            }
            if (((input & (1<<PC4)) == 0)){
                debounce(PC4);
                state = RUN;
            }
        }

        // If necessary write time to LCD
        if (state == RUN || state == STARTRUN){
            lcd_moveto(0, 0);
            char num[15];
            snprintf(num, 15, "%d%d.%d", tens, ones, tenths);
            lcd_stringout(num);
        }
    }

    return 0;   /* never reached */
}

/* ----------------------------------------------------------------------- */

void debounce(uint8_t bit)
{
    // Add code to debounce input "bit" of PINC
    // assuming we have sensed the start of a press.
    char pressed = (PINC & (1 << bit));
    if(pressed == 0){
        _delay_ms(5);
        while((PINC & (1 << bit)) == 0) { }
        _delay_ms(5);
    }
}

/* ----------------------------------------------------------------------- */

void timer1_init(void)
{
    // Add code to configure TIMER1 by setting or clearing bits in
    // the TIMER1 registers.

    // Set to CTC mode
    TCCR1B |= (1 << WGM12);

    // Enable Timer Interrupt
    TIMSK1 |= (1 << OCIE1A);

    OCR1A = 25000;
    TCCR1B |= (1 << CS11) | (1 << CS10);
}

ISR(TIMER1_COMPA_vect)
{
    // Increment the time
    if (state == RUN || state == LAP || state == STARTRUN){
        // PORTC ^= (1 << PC5);    // Flip PC5 each time ISR is run
        tenths++;
        if (tenths > 9){
            ones++;
            tenths = 0;
        }
        if (ones > 9){
            tens++;
            ones = 0;
        }
        if (tens > 5){
            tenths = 0;
            ones = 0;
            tens = 0;
        }
    }
}