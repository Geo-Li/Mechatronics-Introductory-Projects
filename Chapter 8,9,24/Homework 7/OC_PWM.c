#include "nu32dip.h"

#define NUMSAMPS 1000

static volatile int Waveform[NUMSAMPS];

void makeWaveform(int PR3);

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) { // _TIMER_2_VECTOR = 8
    static int counter = 0; // initialize counter once
    // insert line(s) to set OC1RS
    OC1RS = Waveform[counter];
    counter++; // add one to counter every time ISR is entered
    if (counter == NUMSAMPS) {
        counter = 0; // roll the counter over when needed
    }
    // insert line to clear interrupt flag
    IFS0bits.T2IF = 0;
}

int main(void) {
    NU32DIP_Startup();

    // For question 2, we are using Timer3
    // Set the prescaler to 1, which is 0b000 for Timer3
    T3CONbits.TCKPS = 0b000;
    PR3 = 2400;
    TMR3 = 0;
    OC1CONbits.OCM = 0b110;
    OC1CONbits.OCTSEL = 1;
    // Here, our duty cycle is 75%
    OC1RS = 1800;
    OC1R = 1800;
    T3CONbits.ON = 1;
    // Put OC1 on B15
    RPB15Rbits.RPB15R = 0b0101;
    OC1CONbits.ON = 1;
    
    makeWaveform(PR3);

    // For question 3, we are using Timer2
    // Initialize the interrupt
    __builtin_disable_interrupts(); // step 2: disable interrupts
    T2CONbits.TCKPS = 0b000;
    PR2 = 48000; // rollover at 40,000; 48MHz/48k = 1 kHz
    TMR2 = 0; // set the timer count to zero
    T2CONbits.ON = 1; // turn the timer on
    IPC2bits.T2IP = 5; // INT step 4: priority for Timer2 to be 5
    IFS0bits.T2IF = 0; // INT step 5: clear interrupt flag
    IEC0bits.T2IE = 1; // INT step 6: enable interrupt
    __builtin_enable_interrupts(); // INT step 7: enable interrupts at CPU

    while (1) {}
    return 0;
}

void makeWaveform(int PR3) {
    int i = 0, center = (PR3+1)/2, A = PR3/4;
    for (i = 0; i < NUMSAMPS; ++i) {
        if ( i < NUMSAMPS/2) {
            Waveform[i] = center + A;
        } else {
            Waveform[i] = center - A;
        }
    }
}

