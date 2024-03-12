#include <xc.h>          // Load the proper header for the processor

int main(void) {
  TRISBCLR = 0x30; // Make TRIS for B4 and B5 0, 
  LATBbits.LATB4 = 0;    // Turn GREEN on and YELLOW off.  These pins sink current
  LATBbits.LATB5 = 0;    // on the NU32DIP, so "high" (1) = "off" and "low" (0) = "on"
  return 0;
}

