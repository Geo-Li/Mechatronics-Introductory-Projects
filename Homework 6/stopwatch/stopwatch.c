#include "NU32DIP.h" // constants, funcs for startup and UART

#define BUFFER_SIZE 100

volatile unsigned int timer = 0;
volatile unsigned int state = 0;

void __ISR(_EXTERNAL_0_VECTOR, IPL2SOFT) Ext0ISR(void) { // step 1: the ISR
    // _CP0_SET_COUNT(0);
    char buffer[BUFFER_SIZE];
    unsigned int begin_timer = _CP0_GET_COUNT();
    if (begin_timer - timer >= 2400) {
        if (!state) {
            char first_press[BUFFER_SIZE];
            sprintf(first_press, "Press the USER button again to stop the timer.\r\n");
            NU32DIP_WriteUART1(first_press);
        } else {
            // We have detected the sequencial presses,
            // now, print the time between presses
            char between_time[BUFFER_SIZE];
            float time_passed = (begin_timer-timer)/24000000.0;
            sprintf(between_time, "%f seconds elapsed.\r\n", time_passed);
            NU32DIP_WriteUART1(between_time);
        }
        state += 1;
        // sprintf(buffer, "the state is: %i\r\n", state);
        // NU32DIP_WriteUART1(buffer);
    }
    timer = begin_timer;
    IFS0bits.INT0IF = 0; // clear interrupt flag IFS0<3>
}

int main(void) {
    NU32DIP_Startup(); // cache on, min flash wait, interrupts on, LED/button init, UART init
    __builtin_disable_interrupts(); // step 2: disable interrupts
    INTCONbits.INT0EP = 0; // step 3: INT0 triggers on falling edge
    IPC0bits.INT0IP = 2; // step 4: interrupt priority 2
    IPC0bits.INT0IS = 1; // step 4: interrupt priority 1
    IFS0bits.INT0IF = 0; // step 5: clear the int flag
    IEC0bits.INT0IE = 1; // step 6: enable INT0 by setting IEC0<3>
    __builtin_enable_interrupts(); // step 7: enable interrupts
    // Connect RD7 (USER button) to INT0 (D0)
    int seconds = 0;
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "Timer has been reset, now is second %i\r\n", seconds);
    NU32DIP_WriteUART1(buffer);
    unsigned int curr_timer = _CP0_GET_COUNT();
    while(1) {
        while (_CP0_GET_COUNT() - curr_timer < 24000000) {}
        seconds += 1;
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "Now is second %i\r\n", seconds);
        NU32DIP_WriteUART1(buffer);
        curr_timer = _CP0_GET_COUNT();
        ; // do nothing, loop forever
    }
    return 0;
}