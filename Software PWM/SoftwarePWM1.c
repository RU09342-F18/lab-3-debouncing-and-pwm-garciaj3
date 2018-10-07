/* Rowan University
 * File: SoftwarePWM1.c
 * Author: Jan Garcia
 * Course: Intro to Embedded Systems
 * Section: 1
 * Creation Data: 10/5/18
 * Lab 3
 * Application - Software PWM
 * Board - MSP430G2553
 *
 * Description: The purpose of this lab is to utilize the MSP430G2553
 * and create a program to effectively implement a 1kHz PWM signal with a duty cycle
 * between 0% and 100%. The PWM should control the state of an LED at a 50% duty cycle.
 * Upon pressing one of buttons, the duty cycle of the LED should increase by 10%.
 * Once the LED reaches 100%, the duty cycle should go back to 0% on the next button press.
 * There is also an implement of an other LED to light up when the Duty Cycle button is
 * depressed and turns back off when it is let go.
 *
 * This program also uses timer modules and interrupts to manage the debouncing in the background.
 * The Port1 interrupt is called when it detects a falling edge from HitoLow on the pin
 * connected to the switch P1.3. This begins the first switch case statement
 * by enabling the first - Timer0 and the second switch case statement in the
 * Timer0 interrupt that initiates the PWM. Then, the Port1 interrupt triggers again
 * and starts the second timer for the debounce switch. Finally, the last switch case
 * statement is triggered in the Timer0 interrupt and begins to turn the LED OFF from HitoLow.
 * Finally, the whole process is then repeated for the next duty cycle.
 *
 */

#include <msp430.h> 


int pwm = 500; //initialize pwm variable
int state = 0; //initialize state variable

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // disables watchdog timer

    P1DIR &= ~BIT3; //Set P1.3 as input
    P1REN |= BIT3; //Enable pullup/pulldown resistor for P1.3
    P1OUT |= BIT3; //Set P1.3 with pull up resistor
    P1IE |= BIT3; //Enable interrupt on P1.3
    P1IES |= BIT3; //Set interrupt flag on HitoLow edge on P1.3
    P1IFG &= ~BIT3; // button interrupt flag cleared


    P1DIR |= BIT6; // Set P1.6 as output
    P1DIR |= BIT0; // Set P1.0 as output
    P1OUT &= ~BIT6; //Turn off LED P1.6
    P1OUT &= ~BIT0; //Turn off LED P1.0

    TA0CCR0 = 1000; // PWM Period
    TA0CTL = TASSEL_2 + MC_1 + TACLR; // start Timer0 with SMCLK, UP mode, clear timer
    TA1CCTL0 = CCIE; // CCR1 interrupt enabled
    TA1CCR0 = 50000; // Overflow time = 10ms for debouncing

    __enable_interrupt(); //Enable interrupts

    while(1)
    {

        if(TA0R <= pwm) P1OUT ^= BIT6; // pwm/1000 * 100% Turns the LED P1.6 on
        else if (TA0R > pwm) P1OUT &= ~BIT6; // Turns the LED P1.6 off

        if(!(P1IN & BIT3)) P1OUT ^= BIT0; // Toggle LED 1.0 on
        else {P1OUT &= ~BIT0;} // Toggle LED 1.0 off
    }
}
    #pragma vector = PORT1_VECTOR
    __interrupt void P1_ISR(void)
    {
        switch(state)
        {

        case 0: // Off to On
            TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK, continuous mode, clear timer
            P1IE &= ~BIT3; // Disable interrupts for P1.3
            P1IFG &= ~BIT3; // Clear flag P1.3
            break;
        case 1: //On to Off
            TA1CTL = TASSEL_2 + MC_1 + TACLR; // SMCLK, continuous mode, clear timer
            P1IE &= ~BIT3; // Disable interrupts for P1.3
            P1IFG &= ~BIT3;// Clear flag P1.3
            break;
        }
    }
    #pragma vector=TIMER1_A0_VECTOR
    __interrupt void Timer_A0 (void)
    {
        switch(state)
        {
        case 0: // On
            if(pwm < 1000) pwm += 100; //If duty cycle is not at 100% then add 10% brightness
            else pwm = 0; // Else reset to 0

            P1IE |= BIT3; // Enabled Interrupt P1.3
            P1IES &= ~BIT3; // Lo-to-hi interrupt edge
            TA1CTL &= ~TASSEL_2; // Stop timer
            TA1CTL |= TACLR; // Clear timer
            state = 1;
            break;
        case 1:// Off
             P1IE |= BIT3; //Enable Interrupts for P1.3
             P1IFG &= ~BIT3; //Clear interrupt flag P1.3
             P1IES |= BIT3;//high to low interrupt edge
             TA1CTL &= ~TASSEL_2 + TACLR; //Stop TimerA1, clear timer
             state = 0; //go to port 2 state 0
             break;
        }
}



