/* Rowan University
 * File: HardwarePWM1.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/5/18
 * Lab 3
 * Application - Hardware PWM
 * Board - MSP430G2553
 *
 * Description: The purpose of this lab is to utilize the MSP430G2553
 * and create a program to effectively implement a a 1kHz PWM signal with a duty cycle
 * between 0% and 100%. The PWM should control the state of an LED at a 50% duty cycle.
 * Upon pressing one of buttons, the duty cycle of the LED should increase by 10%.
 * Once the LED reaches 100%, the duty cycle should go back to 0% on the next button press.
 * There is also an implement of an other LED to light up when the Duty Cycle button is
 * depressed and turns back off when it is let go.
 *
 * This program is implemented as hardware PWM which requires the use of the OUTMOD hardware feature.
 * The OUTMOD7 is used to allow a reset/set functionality. By setting OUTMOD7 to CCR1,
 * the microprocessor will carry out the functions implemented in the software PWM.
 * The CCR1 register will count up to its value, trigger a reset, and then toggle once the value in CCR0 is reached.
 * Essentially, the two registers are compared through the OUTMOD7 hardware functionality.
 */

#include <msp430.h> 

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    P1DIR |= BIT0; // Set P1.0 to output
    P1DIR |= BIT6; // Set P1.6 to output
    P1SEL |= (BIT6); // Set port 1 for output

    P1OUT &= ~BIT0; // Initialize LED off
    P1DIR &= ~BIT3; // Set P1.3 direction to input
    P1REN |= BIT3; // Enable resistor on P1.3
    P1OUT |= BIT3; // Set resistor to pull up

    P1IE |= BIT3; // Enable interrupt on P1.3
    P1IES |= BIT3; // Low to high interrupt edge
    P1IFG &= ~BIT3; // Clear P1.3 interrupt flags

    TA0CTL = TASSEL_2 + MC_1 + ID_1; // UP mode, divide by 2
    TA0CCR0 = 1000; // Set period
    TA0CCR1 = 100; // Set duty cycle
    TA0CCTL1 = OUTMOD_7; // Set timer to set/reset mode

    __enable_interrupt();
    while(1); // Infinite while loop
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IES & BIT3)
        if (TA0CCR1<1000) // If duty cycle is less than 100%
            TA0CCR1+=100; // Increment duty cycle by 10%
    else if (TA0CCR1 >=1000) // If duty cycle is 100% or more
        TA0CCR1 = 0; // Reset duty cycle
    P1IES ^= BIT3; // Interrupt edge
    P1OUT ^= BIT0; // Toggle LED
    P1IFG &= ~BIT3; // Clear P1.3 interrupt flags
}
