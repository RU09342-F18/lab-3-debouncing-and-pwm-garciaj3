/* Rowan University
 * File: HardwarePWM1.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/5/18
 * Lab 3
 * Application - Hardware PWM
 * Board - MSP430FR2311
 *
 * Description: The purpose of this lab is to utilize the MSP430FR2331
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
 * This program is similar to the MSP4302553 but it differs from ports and pins.
 */

#include <msp430.h> 


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
    P1DIR |= BIT0; // Set P1.0 to output
    P2DIR |= BIT0; // Set P2.0 to output
    P2SEL0 |= (BIT0); // Set P2.0 as output
    P2SEL1 &= ~BIT0;
    P1DIR &= ~BIT1; // Set pin 1.1 direction to input
    P1REN |= BIT1; // Enable resistor on pin 1.1
    P1OUT |= BIT1; // Set resistor to pull up
    P1IE |= BIT1; // Enable P1.1 interrupt
    P1IES |= BIT1; // Set interrupt edge to low
    P1IFG &= ~BIT1; // Clear P1.1 interrupt flags
    TB1CTL = TBSSEL_2 + MC_1 + ID_1; // Set Timer B1 to SMLCK, up mode, internal divider
    TB1CCR0 = 1000; // Set period
    TB1CCR1 = 500; // Set duty cycle to 50%
    TB1CCTL1 = OUTMOD_7; // Set timer to set/reset mode
    TB0CCTL0 = CCIE; // Enable CCR interrupt
    TB0CCR0 = 10000; // Set timer overflow for debounce
    TB0CTL = TBSSEL_2 + MC_0; // Timer B0 to SMCLK, stop mode
    __bis_SR_register(GIE); // LPM0 with interrupts enabled
    while(1); // Infinite while loop
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    P1IE &= ~BIT1; // Disable P1.1 interrupt
    if(P1IES & BIT1)
    {
            if (TB1CCR1<1000) { // If duty cycle is less than 100%
                TB1CCR1+=100; // Increment duty cycle by 10%
            } else if (TB1CCR1 >=1000) { // If duty cycle is 100% or more
                TB1CCR1 = 0; // Reset duty cycle
            }
    }
           P1IES ^= BIT1; // Toggle edge
           P1OUT ^= BIT0; // Toggle LED
           P1IFG &= ~BIT1; // Clear P1.1 flags
           TB0CTL = TBSSEL_2 + MC_1; // Connect timer b0 to SMCLK in up mode
}

// Debounce interrupt
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B (void)
{
    TB0CTL = MC_0; // Stop timer
    TB0R = 0; // Reset timer
    P1IE |= BIT1; // Enable P1.1 interrupt
    P1IFG &= ~BIT1; // Clear P1.1 interrupt flags
}
