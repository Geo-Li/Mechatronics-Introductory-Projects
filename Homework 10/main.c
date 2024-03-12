#include "NU32DIP.h"
#include "ENCODER.h"
#include "UTILITIES.h"
#include "INA219.h"
#include "OC_PWM.h"

#define BUFFER_SIZE 50
#define NUMSAMPS 100
#define REFERENCE_CURRENT 200
#define EINTMAX 500
#define UMAX 100

static volatile int dutyCycle = 0;
static volatile float curr_kp = 0.1, curr_ki = 0.0;
static volatile float eint = 0, pos_eint = 0, pos_eprev = 0;
static volatile float pos_kp = 0.1, pos_ki = 0, pos_kd = 0.1;
static volatile int trajectoryLength = 0;
static volatile float REFposition = 0;
static volatile float posREFcurrent = 0;
// define arrays to store data points for plotting
static volatile float current_REF_array[NUMSAMPS];
static volatile float current_ACT_array[NUMSAMPS];
static volatile float trajectory_REF_array[NUMSAMPS*10];
static volatile float trajectory_ACT_array[NUMSAMPS*10];


void reset(enum mode_t mode);
int get_motor_count();
float adjust_value(float val, float limit);
float current_adjustment(int pos_flag, float error, float kp, float ki, float kd);
void load_trajectory();
void reset_float_array(float arr[], int length);


void __ISR(_TIMER_2_VECTOR, IPL5SOFT) CURRENT_Controller(void) {
    static int count = 0;
    static int current_sign = 0;

    switch (get_mode()) {
        case IDLE:
        {
            OC1RS = 0;
            break;
        }
        case PWM:
        {
            if (dutyCycle > 0) {
                LATBbits.LATB14 = 1; // spin clockwise
                OC1RS = dutyCycle * PR3 / 100.0; // duty cycle = OC1RS/(PR3+1) = 25%
            } else if (dutyCycle < 0) {
                LATBbits.LATB14 = 0; // spin counter-clockwise
                OC1RS = -dutyCycle * PR3 / 100.0; // duty cycle = OC1RS/(PR3+1) = 25%
            }
            break;
        }
        case ITEST:
        {
            float REFcurrent = current_sign * REFERENCE_CURRENT;
            float ACTcurrent = INA219_read_current();

            current_REF_array[count] = REFcurrent;
            current_ACT_array[count] = ACTcurrent;

            float error = REFcurrent - ACTcurrent;

            if (error > 0) {
                LATBbits.LATB14 = 0;
            } else if (error < 0) {
                LATBbits.LATB14 = 1;
            }

            float u = current_adjustment(0, error, curr_kp, curr_ki, 0);
            
            OC1RS = (unsigned int) (u * PR3 / 100.0);
            count++;
            if (count == 25 || count == 50 || count == 75) {
                current_sign = !current_sign;
            } else if (count == 99) {
                reset(IDLE);
                count = 0;
                break;
            }
            break;
        }
        case HOLD:
        {
            // Invert the spining direction based on the adjusted 
            // reference position current
            if (posREFcurrent > 0) {
                LATBbits.LATB14 = 1;
            } else if (posREFcurrent < 0) {
                LATBbits.LATB14 = 0;
            }
            float ACTcurrent = INA219_read_current();
            float error = posREFcurrent - ACTcurrent;

            float u = current_adjustment(0, error, curr_kp, curr_ki, 0);
            OC1RS = (unsigned int) (u * PR3 / 100.0);
            break;
        }
        case TRACK:
        {
            // Invert the spining direction based on the adjusted 
            // reference position current
            if (posREFcurrent > 0) {
                LATBbits.LATB14 = 1;
            } else if (posREFcurrent < 0) {
                LATBbits.LATB14 = 0;
            }
            float ACTcurrent = INA219_read_current();
            float error = posREFcurrent - ACTcurrent;

            float u = current_adjustment(0, error, curr_kp, curr_ki, 0);
            OC1RS = (unsigned int) (u * PR3 / 100.0);
            break;
        }
        default:
        {
            // OC1RS = 0;
            break;
        }
    }
    IFS0bits.T2IF = 0; // Clear interrupt flag
}


void __ISR(_TIMER_4_VECTOR, IPL6SOFT) POSITION_Controller(void) {
    static int count = 0;

    switch (get_mode()) {
        case HOLD:
        {
            float ACTposition = get_motor_count() / 384.0 * 360.0;

            float error = REFposition - ACTposition;

            float u = current_adjustment(1, error, pos_kp, pos_ki, pos_kd);
            posREFcurrent = u;
            break;
        }
        case TRACK:
        {
            REFposition = trajectory_REF_array[count];
            float ACTposition = get_motor_count() / 384.0 * 360.0;

            trajectory_ACT_array[count] = ACTposition;
            float error = REFposition - ACTposition;

            float u = current_adjustment(1, error, pos_kp, pos_ki, pos_kd);
            posREFcurrent = u;

            count++;
            // Reset variables if we have done comparing with the reference array
            if (count == trajectoryLength) {
                // Set this to HOLD when HOLD is done
                reset(HOLD);
                count = 0;
            }
            break;
        }
        default:
        {
            break;
        }
    }
    IFS0bits.T4IF = 0; // Clear interrupt flag
}


int main(void) {
    char start[BUFFER_SIZE];
    sprintf(start, "Program Started\r\n"); // print every time restart the PIC
    NU32DIP_WriteUART1(start);

    NU32DIP_Startup();  // Initialize PIC
    UART2_Startup();    // Initialize PICO
    LATBbits.LATB5 = 1; // Turn YELLOW LED off

    // Initialize the interrupt
    __builtin_disable_interrupts();
    // in future, initialize modules or peripherals here
    reset(IDLE);
    INA219_Startup();
    PWM_Startup();
    __builtin_enable_interrupts();

    while (1) {
        // Read the char from screen
        char command[BUFFER_SIZE];
        NU32DIP_ReadUART1(command, BUFFER_SIZE);
        LATBbits.LATB5 = 1; // clear the error LED
        switch (command[0]) {
            // Read current sensor (ADC counts)
            case 'a':
            {
                // There is no ADC we need to read in this project,
                // so I leave it blank here
                break;
            }
            // Read current sensor (mA)
            case 'b':
            {
                float current = INA219_read_current();
                char message[BUFFER_SIZE];
                sprintf(message, "Current is: %f\r\n", current);
                NU32DIP_WriteUART1(message);
                break;
            }
            // Read encoder (counts)
            case 'c':
            {
                int p = get_motor_count();
                char message[BUFFER_SIZE];
                sprintf(message, "Motor is now in count: %d\r\n", p);
                NU32DIP_WriteUART1(message);
                break;
            }
            // Read encoder (deg)
            case 'd':
            {
                // For dummy command
                /*
                int n = 0;
                char input[BUFFER_SIZE];
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%d", &n);
                sprintf(input, "%d\r\n", n + 1); // return the number + 1
                NU32DIP_WriteUART1(input);
                break;
                */
                float p = get_motor_count();
                char message[BUFFER_SIZE];
                sprintf(message, "Motor is now in degree: %f\r\n", (p / 384.0 * 360.0));
                NU32DIP_WriteUART1(message);
                break;
            }
            // Reset encoder count
            case 'e':
            {
                WriteUART2("b");
                char message[BUFFER_SIZE];
                sprintf(message, "Motor has been reset\r\n");
                NU32DIP_WriteUART1(message);
                break;
            }
            // Set PWM (-100 to 100)
            case 'f':
            {
                set_mode(PWM);
                char input[BUFFER_SIZE];
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%d", &dutyCycle);
                sprintf(input, "Duty cycle has been set to: %d\r\n", dutyCycle); // print the duty cycle
                NU32DIP_WriteUART1(input);
                break;
            }
            // Set current gains
            case 'g':
            {
                char input[BUFFER_SIZE];
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%f %f", &curr_kp, &curr_ki);
                break;
            }
            // Get current gains
            case 'h':
            {
                char message[BUFFER_SIZE];
                sprintf(message, "Current control kp: %f \tki: %f\r\n", curr_kp, curr_ki); // print kp and ki
                NU32DIP_WriteUART1(message);
                break;
            }
            // Set position gains
            case 'i':
            {
                char input[BUFFER_SIZE];
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%f %f %f", &pos_kp, &pos_ki, &pos_kd);
                break;
            }
            // Get position gains
            case 'j':
            {
                char message[BUFFER_SIZE];
                sprintf(message, "Position control kp: %f \tki: %f \tkd: %f\r\n", pos_kp, pos_ki, pos_kd); // print pos_kp
                NU32DIP_WriteUART1(message);
                break;
            }
            // Test current gains
            case 'k':
            {
                __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
                eint = 0;
                __builtin_enable_interrupts();  // only 2 simple C commands while ISRs disabled
                
                set_mode(ITEST);
                // Wait till the interrupt is done and all ITEST mode is switched back to IDLE
                while (get_mode() == ITEST) {}

                // Provide number of data points in the array to client
                char message[BUFFER_SIZE];
                sprintf(message, "%d\r\n", NUMSAMPS);
                NU32DIP_WriteUART1(message);

                for (int i=0; i<NUMSAMPS; i++) {
                    // print format: reference current [space] actual current
                    sprintf(message, "%f %f\r\n", current_REF_array[i], current_ACT_array[i]);
                    NU32DIP_WriteUART1(message);
                }
                break;
            }
            // Go to angle (deg)
            case 'l':
            {
                char input[BUFFER_SIZE];
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%f", &REFposition);

                set_mode(HOLD);
                break;
            }
            // Load step trajectory
            case 'm':
            {
                load_trajectory();
                break;
            }
            // Load cubic trajectory
            case 'n':
            {
                load_trajectory();
                break;
            }
            // Execute trajectory
            case 'o':
            {
                __builtin_disable_interrupts(); // keep ISR disabled as briefly as possible
                eint = 0;
                pos_eint = 0;
                pos_eprev = 0;
                WriteUART2("b");
                __builtin_enable_interrupts();  // set ISR back to enabled
                
                set_mode(TRACK);
                while (get_mode() == TRACK) {}

                char message[BUFFER_SIZE];
                sprintf(message, "%d\r\n", trajectoryLength);
                NU32DIP_WriteUART1(message);

                for (int i=0; i<trajectoryLength; i++) {
                    // print format: reference current [space] actual current
                    sprintf(message, "%f %f\r\n", trajectory_REF_array[i], trajectory_ACT_array[i]);
                    NU32DIP_WriteUART1(message);
                }

                break;
            }
            // Unpower the motor
            case 'p':
            {
                reset(IDLE);
                break;
            }
            // Quit
            case 'q':
            {
                // handle q for quit. Later you may want to return to IDLE mode here.
                reset(IDLE);
                break;
            }
            // Get mode
            case 'r':
            {
                char message[BUFFER_SIZE];
                sprintf(message, "%d\r\n", get_mode()); // print the current mode
                NU32DIP_WriteUART1(message);
                break;
            }
            // Add two numbers
            case 'x':
            {
                int n1, n2;
                char input[BUFFER_SIZE];
                // sprintf(input, "Enter two numbers for addition:\r\n"); // return the number1 + number2
                // NU32DIP_WriteUART1(input);
                NU32DIP_ReadUART1(input, BUFFER_SIZE);
                sscanf(input, "%d %d", &n1, &n2);
                sprintf(input, "Addition result: %d\r\n", n1 + n2); // print the number1 + number2
                NU32DIP_WriteUART1(input);
                break;
            }
            // Unkown command
            default:
            {
                LATBbits.LATB5 = 0; // turn on LED2 to indicate an error
                break; 
            }
        }
    }
    // Return 0 if the main program executed successfully
    return 0;
}


void reset(enum mode_t mode) {
    set_mode(mode);
    dutyCycle = 0;
    eint = 0;
    pos_eint = 0;
    pos_eprev = 0;
}


int get_motor_count() {
    WriteUART2("a");
    while (!get_encoder_flag()) {}
    set_encoder_flag(0);
    return get_encoder_count();
}


float adjust_value(float val, float limit) {
    if (val > limit) {
        val = limit;
    } else if (val < -limit) {
        val = -limit;
    }
    return val;
}


float current_adjustment(int pos_flag, float error, float kp, float ki, float kd) {
    if (!pos_flag) {
        // Adjust the actual current to match the reference current
        eint += error; // error sum
        eint = adjust_value(eint, EINTMAX);
        float u = (kp * error) + (ki * eint);
        if (u < 0) {
            u = -u;
        }
        u = adjust_value(u, UMAX);
        return u;
    } else {
        // Adjust the actual current to match the position reference current
        pos_eint += error;
        float pos_ederiv = error - pos_eprev;
        pos_eprev = error;
        float u = (kp * error) + (ki * pos_eint) + (kd * pos_ederiv);
        // Do some limitation check if you need one here
        return u;
    }
}


void load_trajectory() {
    char input[BUFFER_SIZE];
    NU32DIP_ReadUART1(input, BUFFER_SIZE);
    sscanf(input, "%d", &trajectoryLength);

    // Store the data sent from client to trajectory_STEP_array
    for (int i=0; i<trajectoryLength; i++) {
        NU32DIP_ReadUART1(input, BUFFER_SIZE);
        sscanf(input, "%f", &trajectory_REF_array[i]);
    }

    // Send the data in trajectory_STEP_array back to client
    // for (int i=0; i<trajectoryLength; i++) {
    //     sprintf(input, "%f\r\n", trajectory_REF_array[i]);
    //     NU32DIP_WriteUART1(input);
    // }
}


void reset_float_array(float arr[], int length) {
    for (int i=0; i<length; i++) {
        arr[i] = 0.0;
    }
}

