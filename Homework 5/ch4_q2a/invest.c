#include "nu32dip.h"
#include <stdio.h>

#define MAX_YEARS 100
#define BUFFER_SIZE 100

typedef struct {
    double inv0;
    double growth;
    int years;
    double invarray[MAX_YEARS+1];
} Investment;


int getUserInput(Investment *invp);
void calculateGrowth(Investment *invp);
void sendOutput(double *arr, int years);

int main(void) {
    NU32DIP_Startup();
    Investment inv;
    while(getUserInput(&inv)) {
        inv.invarray[0] = inv.inv0;
        calculateGrowth(&inv);
        sendOutput(inv.invarray, inv.years);
    }
    return 0;
}

void calculateGrowth(Investment *invp) {
    int i;
    for (i = 1; i <= invp->years; i= i + 1) {
        invp->invarray[i] = invp->growth * invp->invarray[i-1];
    }
}

int getUserInput(Investment *invp) {
    int valid;
    // convert printf to sprintf
    char prompt[BUFFER_SIZE];
    sprintf(prompt, "Enter investment, growth rate, number of yrs (up to %d): ",MAX_YEARS);
    NU32DIP_WriteUART1(prompt);
    // convert scanf to sscanf
    char input[BUFFER_SIZE];
    NU32DIP_ReadUART1(input, BUFFER_SIZE);
    sscanf(input, "%lf %lf %d", &(invp->inv0), &(invp->growth), &(invp->years));
    valid = (invp->inv0 > 0) && (invp->growth > 0) && (invp->years > 0) && (invp->years <= MAX_YEARS);
    // convert printf to sprintf
    char check[BUFFER_SIZE];
    sprintf(check, "Valid input? %d\r\n",valid);
    NU32DIP_WriteUART1(check);
    if (!valid) {
        char invalid[BUFFER_SIZE];
        sprintf(invalid, "Invalid input; exiting.\r\n");
        NU32DIP_WriteUART1(invalid);
    }
    return(valid);
}

void sendOutput(double *arr, int yrs) {
    int i;
    // convert printf to sprintf
    char outstring[100], result[BUFFER_SIZE];
    sprintf(result, "\r\nRESULTS:\r\n\r\n");
    NU32DIP_WriteUART1(result);
    for (i=0; i<=yrs; i++) {
        sprintf(outstring,"Year %3d: %10.2f\r\n",i,arr[i]);
        NU32DIP_WriteUART1(outstring);
    }
    // convert printf to sprintf
    char newline[BUFFER_SIZE];
    sprintf(newline, "\r\n");
    NU32DIP_WriteUART1(newline);
}