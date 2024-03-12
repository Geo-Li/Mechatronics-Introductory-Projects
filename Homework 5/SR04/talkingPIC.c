#include "nu32dip.h"          // constants, funcs for startup and UART
#include "sr04.h"
#include <stdio.h>

#define MAX_MESSAGE_LENGTH 200
#define BUFFER_SIZE 100

int main(void) {
  char message[MAX_MESSAGE_LENGTH];
  
  NU32DIP_Startup(); // cache on, interrupts on, LED/button init, UART init
  SR04_Startup();
  while (1) {
    unsigned int timer = _CP0_GET_COUNT();
    // NU32DIP_ReadUART1(message, MAX_MESSAGE_LENGTH);  // get message from computer
    char distance[BUFFER_SIZE];
    float meters = SR04_read_meters();
    sprintf(distance, "Distance detected: %f\r\n",meters);
    NU32DIP_WriteUART1(distance);                     // send message back
    // NU32DIP_WriteUART1("\r\n");                      // carriage return and newline
    // NU32DIP_GREEN = !NU32DIP_GREEN;                       // toggle the LEDs
    // NU32DIP_YELLOW = !NU32DIP_YELLOW;
    while (_CP0_GET_COUNT() - timer < 24000000/4){}
  }
  return 0;
}
