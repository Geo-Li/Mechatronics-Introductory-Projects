#include "nu32dip.h"

#define NUMSAMPS 1000 // number of points in waveform
#define PLOTPTS 200 // number of data points to plot
#define DECIMATION 10 // plot every 10th point
#define SAMPLE_TIME 6 // 24MHz*250ns
#define EINTMAX 1000

static volatile int Waveform[NUMSAMPS]; // waveform
static volatile int ADCarray[PLOTPTS]; // measured values to plot
static volatile int REFarray[PLOTPTS]; // reference values to plot
static volatile int StoringData = 0; // if this flag = 1, currently storing plot data
static volatile float Kp = 0, Ki = 0; // control gains
static volatile int eint = 0;

void makeWaveform(int freq);
void ADC_Startup();
unsigned int adc_sample_convert(int pin);

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Controller(void) {
    static int counter = 0; // initialize counter once
    static int plotind = 0; // index for data arrays; counts up to PLOTPTS
    static int decctr = 0; // counts to store data one every DECIMATION
    static int adcval = 0; // 

    int pin = 0b001;
    adcval = adc_sample_convert(pin);

    // insert line(s) to set OC1RS
    float e = Waveform[counter] - adcval;
    eint = eint + e; // error sum
    if (eint > EINTMAX) { // ADDED: integrator anti-windup
        eint = EINTMAX;
    } else if (eint < -EINTMAX) { // ADDED: integrator anti-windup
        eint = -EINTMAX;
    }
    float u = Kp * e + Ki * eint;
    float unew = u + 50.0;
    unew = u + 50.0;
    if (unew > 100.0) {
        unew = 100.0;
    } else if (unew < 0.0) {
        unew = 0.0;
    }
    // OC1RS = Waveform[counter];
    OC1RS = (unsigned int) ((unew/100.0) * PR3);

    if (StoringData) {
        decctr++;
        if (decctr == DECIMATION) { // after DECIMATION control loops,
            decctr = 0; // reset decimation counter
            ADCarray[plotind] = adcval; // store data in global arrays
            REFarray[plotind] = Waveform[counter];
            plotind++; // increment plot data index
        }
        if (plotind == PLOTPTS) { // if max number of plot points plot is reached,
            plotind = 0; // reset the plot index
            StoringData = 0; // tell main data is ready to be sent to MATLAB
        }
    }
    counter++; // add one to counter every time ISR is entered
    if (counter == NUMSAMPS) {
        counter = 0; // rollover counter over when end of waveform reached
    }
    // insert line to clear interrupt flag
    IFS0bits.T2IF = 0;
}

int main(void) {
    // Initialize the program
    NU32DIP_Startup();
    ADC_Startup();

    char message[100]; // message to and from MATLAB
    float kptemp = 0, kitemp = 0; // temporary local gains

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
    
    makeWaveform(0);

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

    while (1) {
        NU32DIP_ReadUART1(message, sizeof(message)); // wait for a message from MATLAB
        sscanf(message, "%f %f" , &kptemp, &kitemp);
        __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
        Kp = kptemp; // copy local variables to globals used by ISR
        Ki = kitemp;
        eint = 0;
        __builtin_enable_interrupts(); // only 2 simple C commands while ISRs disabled
        StoringData = 1; // message to ISR to start storing data
        while (StoringData) { // wait until ISR says data storing is done
            ; // do nothing
        }
        for (int i=0; i<PLOTPTS; i++) { // send plot data to MATLAB
            // when first number sent = 1, MATLAB knows we’re done
            sprintf(message, "%d %d %d\r\n", PLOTPTS-i, ADCarray[i], REFarray[i]);
            NU32DIP_WriteUART1(message);
        }
    }
    return 0;
}

void makeWaveform(int freq) {
    int i = 0, center = 500, A = 300;
    for (i = 0; i < NUMSAMPS; ++i) {
        if ( i < NUMSAMPS/2) {
            Waveform[i] = center + A;
        } else {
            Waveform[i] = center - A;
        }
    }
}

void ADC_Startup() {
  ANSELAbits.ANSA1 = 1; // AN1 is an adc pin
  AD1CON3bits.ADCS = 1; // ADC clock period is Tad = 2*(ADCS+1)*Tpb =2*2*(1/48000000Hz) = 83ns > 75ns
  AD1CON1bits.ADON = 1;
}

unsigned int adc_sample_convert(int pin) {
  unsigned int elapsed = 0, finish_time = 0;
  AD1CHSbits.CH0SA = pin;
  AD1CON1bits.SAMP = 1;
  elapsed = _CP0_GET_COUNT();
  finish_time = elapsed + SAMPLE_TIME;
  while (_CP0_GET_COUNT() < finish_time)
  {
    ;
  }
  AD1CON1bits.SAMP = 0;
  while (!AD1CON1bits.DONE)
  {
    ;
  }
  return ADC1BUF0;
}

