/* Rowan University
 * File: debouncing2.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/5/18
 * Lab 3
 * Application - Software Debouncing
 * Board - MSP432P401R
 *
 * Description: The purpose of this lab is to utilize the MSP432P401R
 * and create a program to effectively implement a debounced switch
 * control on the state of an LED. This program uses timer modules and
 * interrupts to manage the debouncing in the background.The Port1
 * interrupt is called when it detects a falling edge on the pin
 * connected to the switch P1.1. This begins the Timer0 and every 1 ms,
 * the timer gives an interrupt, and the switch is read. If the switch
 * remains in the same state for 10 checks in a row, then the state of
 * the switch can toggle the LED on P2.2. Finally, the whole process
 * is then repeated for the next switch bounce and the timer with the
 * interrupt flag gets reset.
 *
 */

#include <msp432.h>

int c=0; //initialize counter variable
int state=0; //initialize state variable

void main(void)
{
    // Hold the watchdog

        WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

        // Configuring P1.0 as output and P1.1 (switch) as input with pull-up
        // resistor. Rest of pins are configured as output low.

        P1DIR &= ~BIT1; // Set P1.1 as input GIPO
        P1OUT |= BIT1; // Set P1.1 as pullup resistor
        P1REN |= BIT1; // Enable pull-up resistor (P1.1 output high)
        P1IES |= BIT1; // Interrupt on high-to-low transition
        P1IE |= BIT1; // Enable interrupt for P1.1

        P2SEL0 |= 2; // Default to simple I/O
        P2SEL1 |= 2; // Default to simple I/O


        P2DIR |= BIT2; // P2.2 to output direction
        P2OUT&=~BIT2; // Turn off LED

        P1IFG &= ~BIT1;     // Clear P1.1 interrupt flags

        TA0CCR0 = 1000; //Make the timer count up to 10000, about 1 ms

        // Enable Port P1 interrupts
        NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);   // Interrupt enable P1
        NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);   // Interrupt enable TimerA0

        // Enable global interrupt
        __enable_irq();   // Enable global interrupt
        SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;  // Disable the sleep on exit feature
        __sleep(); // Sleep Mode

        while(1);

    }

    /* Port1 ISR */
    void PORT1_IRQHandler(void)
    {

        c=0; // set counter=0

        //Start Timer0 with SMCLK clock source, UP mode and enable overflow interrupt
            TA0CTL |= TASSEL_2 + // SMCLK
            MC_1 + // Up mode
            TAIE; //Enable overflow interrupt

            state=(P1IN&BIT1); // Save the state of the switch
            P1IE&=~BIT1; // Disable interrupt on P1.1
            P1IFG&=~BIT1; // Reset Port1 interrupt flag

    }
    void TA0_0_IRQHandler(void)
    {
        {
            if(state==((P1IN&BIT1))) // If the state of the LED is the same
            {
                c++; // Increment the counter variable
            }
            else
            {
                c=0; // If not same, reset the counter variable
                state=((P1IN&BIT1)); // And save the present state of the switch
            }
        if(c==10) // If the state has been consistently the same
            {
            if(state==0) // If the switch was pressed
            {
                P2OUT^=BIT2; // Toggle the LED
            }
            P1IE|=BIT1; // Enable interrupt again on P1.1 to detect switch bounce
            TA0CTL=0; // Stop the Timer
            TA0CTL|=TACLR; // Clear the Timer counter
            }

        TA0CTL&=~(TAIFG); // Reset the interrupt flag
        }
    }
