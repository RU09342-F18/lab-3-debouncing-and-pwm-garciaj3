/* Rowan University
 * File: debouncing1.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/5/18
 * Lab 3
 * Application - Software Debouncing
 * Board - MSP430G2553
 *
 * Description: The purpose of this lab is to utilize the MSP430G2553
 * and create a program to effectively implement a debounced switch
 * control on the state of an LED. This program uses timer modules and
 * interrupts to manage the debouncing in the background.The Port1
 * interrupt is called when it detects a falling edge on the pin
 * connected to the switch P1.3. This begins the Timer0 and every 1 ms,
 * the timer gives an interrupt, and the switch is read. If the switch
 * remains in the same state for 10 checks in a row, then the state of
 * the switch can toggle the LED on P1.0. Finally, the whole process
 * is then repeated for the next switch bounce and the timer with the
 * interrupt flag gets reset.
 *
 */

#include <msp430.h> 

int c=0; //initialize counter variable
int state=0; //initialize state variable

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;   // Turn watchdog timer off

    P1DIR&=~BIT3; //Set P1.3 as input
    P1REN|=BIT3; //Enable pullup/pulldown resistor for P1.3
    P1OUT|=BIT3; //Set P1.3 with pull up resistor

    P1IE|=BIT3; //Enable interrupt on P1.3
    P1IES|=BIT3; //Set interrupt flag on HitoLow edge on P1.3

    TACCR0=1000; //Make the timer count up to 10000, about 1 ms
    __enable_interrupt(); //Enable interrupts

    P1DIR|=BIT0; //Set P1.0 as output
    P1OUT&=~BIT0; //Turn off LED

    return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Setclock()
{
    c=0; // set counter=0
    TACTL|=TASSEL_2+MC_1+TAIE; //Start Timer0 with SMCLK clock source, UP mode and enable overflow interrupt
    state=(P1IN&BIT3); //Save the state of the switch
    P1IE&=~BIT3; //Disable interrupt on P1.3
    P1IFG&=~BIT3; // Reset Port1 interrupt flag
}

#pragma vector=TIMER0_A1_VECTOR // Timer0 interrupt
__interrupt void Timer0()
{
        {
            if(state==((P1IN&BIT3))) //If the state of the LED is the same
                c++; //Increment the counter variable
            else
            {
                c=0; //If not same, reset the counter variable
                state=((P1IN&BIT3)); //And save the present state of the switch
            }
        if(c==10) //If the state has been consistently the same
            {
            if(state==0) //If the switch was pressed
                P1OUT^=BIT0; //Toggle the LED
            P1IE|=BIT3; //Enable interrupt again on P1.3 to detect switch bounce
            TACTL=0; //Stop the Timer
            TACTL|=TACLR; //Clear the Timer counter
            }

            TACTL&=~(TAIFG); //Reset the interrupt flag
        }
}
