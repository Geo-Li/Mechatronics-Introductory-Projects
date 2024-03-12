#include "OC_PWM.h"

void PWM_Startup() {
    // Set up timer3 for PWM - 20kHz
    T3CONbits.TCKPS = 0b000;    // Set the prescaler N=1
    PR3 = 2400;                 // rollover at 2400; 48MHz/2400 = 20 kHz
    TMR3 = 0;                   // initial TMR3 count is 0
    OC1CONbits.OCM = 0b110;     // PWM mode without fault pin; other OC1CON bits are defaults
    OC1CONbits.OCTSEL = 1;      // Select Timer3 as output compare  time base
    T3CONbits.ON = 1;           // turn on Timer3
    RPB15Rbits.RPB15R = 0b0101; // Config OC1 on B15
    TRISBbits.TRISB14 = 0;      // Config B14 to be output pin
    OC1CONbits.ON = 1;          // turn on OC1

    // Set up timer2 for ISR - 5kHz
    T2CONbits.TCKPS = 0b000; // Set the prescaler N=1
    PR2 = 9600;              // rollover at 9600; 48MHz/9600 = 5 kHz
    TMR2 = 0;                // set the timer count to zero
    T2CONbits.ON = 1;        // turn the timer on
    IPC2bits.T2IP = 5;       // INT step 4: priority for Timer2 to be 5
    IFS0bits.T2IF = 0;       // INT step 5: clear interrupt flag
    IEC0bits.T2IE = 1;       // INT step 6: enable interrupt

    // Set up timer4 for ISR - 200Hz
    T4CONbits.TCKPS = 0b110; // Set the prescaler N=64
    PR4 = 3750;             // rollover at 15000; 48MHz/64/3750 = 200 Hz
    TMR4 = 0;                // set the timer count to zero
    T4CONbits.ON = 1;        // turn the timer on
    IPC4bits.T4IP = 6;       // INT step 4: priority for Timer4 to be 6
    IFS0bits.T4IF = 0;       // INT step 5: clear interrupt flag
    IEC0bits.T4IE = 1;       // INT step 6: enable interrupt
}
