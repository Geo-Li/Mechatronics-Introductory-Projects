#include "sr04.h"
#include <xc.h>

// macros for the pins
#define TRIG LATBbits.LATB15
#define ECHO PORTBbits.RB14

// initialize the pins used by the SR04
void SR04_Startup(){
    ANSELA = 0; // turn off the analog input functionality that overrides everything else
    ANSELB = 0;
    TRISBbits.TRISB15 = 0; // B15 is TRIG, output from the PIC
    TRISBbits.TRISB14 = 1; // B14 is ECHO, input to the PIC
    TRIG = 0; // initialize TRIG to LOW
}

// trigger the SR04 and return the value in core timer ticks
unsigned int SR04_read_raw(unsigned int timeout){
    // while (ECHO){}
    // turn on TRIG 
    TRIG = 1;
    // wait at least 10us
    unsigned int timer = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() - timer < 240){}
    // turn off TRIG
    TRIG = 0;
    // wait until ECHO is on
    while (!ECHO){}
    // note the value of the core timer
    unsigned int before_timer = _CP0_GET_COUNT();
    // wait until ECHO is off or timeout core ticks has passed
    while (ECHO && _CP0_GET_COUNT() - before_timer < timeout){}
    // note the core timer
    unsigned int after_timer = _CP0_GET_COUNT();
    // return the difference in core times
    return after_timer - before_timer;
}

float SR04_read_meters(){
    // read the raw rs04 value
    unsigned int rs04 = SR04_read_raw(12000000);
    // convert the time to meters, the velocity of sound in air is 346 m/s
    float meters = (float)rs04/24000000.0 * 346;
    // return the distance in meters
    return meters;
}