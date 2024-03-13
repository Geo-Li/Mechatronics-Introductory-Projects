# chapter 28 in python

# sudo apt-get install python3-pip
# python3 -m pip install pyserial
# sudo apt-get install python3-matplotlib

import serial
import matplotlib.pyplot as plt 
from genref import genRef
from read_plot_matrix import read_plot_matrix


def get_line(ser):
    # get the message
    byte_str = ser.read_until(b'\n')
    # print it to the screen
    print('Got back: ' + byte_str.decode('utf-8'))


def write_input(ser, message):
    # set the duty cycle
    input_str = input(message)
    input_endline = input_str + '\n'
    # send the command to the PIC32
    ser.write(input_endline.encode())


def plotRef(ser, pattern, debug=False):
    # generate the reference current
    ref = genRef(pattern)
    # plot the reference current
    t = range(len(ref))
    plt.plot(t,ref,'r*-')
    plt.ylabel('ange in degrees')
    plt.xlabel('index')
    plt.show()
    # send number of data points to PIC
    ser.write((str(len(ref))+'\n').encode())
    # send each data point to PIC
    for i in ref:
        ser.write((str(i)+'\n').encode())
    if debug:
        # check if the data has been sent back from PIC
        for i in t:
            get_line(ser)


def main():
    ser = serial.Serial('/dev/tty.SLAB_USBtoUART', 230400)
    print('Opening port: ')
    print(ser.name + '\n')

    null_listener = ['a', 'p']
    single_output = ['b', 'c', 'd', 'e']
    plotting = ['k', 'o']

    modes_dict = {
        0: "IDLE",
        1: "PWM",
        2: "ITEST",
        3: "HOLD",
        4: "TRACK"
    }

    has_quit = False
    # menu loop
    while not has_quit:
        print("PIC32 MOTOR DRIVER INTERFACE")

        commands = ""
        # display the menu options; this list will grow
        commands += "\ta: Read current sensor (ADC counts)\n"
        commands += "\tb: Read current sensor (mA)\n"
        commands += "\tc: Read encoder (counts)\n"
        commands += "\td: Read encoder (deg)\n"
        commands += "\te: Reset encoder count\n"
        commands += "\tf: Set PWM (-100 to 100)\n"
        commands += "\tg: Set current gains\n"
        commands += "\th: Get current gains\n"
        commands += "\ti: Set position gains\n"
        commands += "\tj: Get position gains\n"
        commands += "\tk: Test current gains\n"
        commands += "\tl: Go to angle (deg)\n"
        commands += "\tm: Load step trajectory\n"
        commands += "\tn: Load cubic trajectory\n"
        commands += "\to: Execute trajectory\n"
        commands += "\tp: Unpower the motor\n"
        commands += "\tr: Get mode\n"
        commands += "\tx: Add Two Numbers\n"
        commands += "\tq: Quit "
        print(commands)

        # read the user's choice
        selection = input('\nENTER COMMAND: ')
        selection_endline = selection+'\n'
        
        # send the command to the PIC32
        ser.write(selection_endline.encode()) # .encode() turns the string into a char array
        
        # take the appropriate action
        if selection in null_listener:
            continue
        elif selection in single_output:
            get_line(ser)
        elif selection == 'f':
            write_input(ser, "Enter the PWM for the motor:\n")
            get_line(ser)
        elif selection == 'g':
            write_input(ser, "Enter kp and ki for the current control (eg. 0.1 0.1):\n")
        elif selection == 'h':
            get_line(ser)
        elif selection == 'i':
            write_input(ser, "Enter kp, ki, and kd for the position control (eg. 0.1 0.1 0.1):\n")
        elif selection == 'j':
            get_line(ser)
        elif selection == 'l':
            write_input(ser, "Enter the desired angle for the motor:\n")
        elif selection == 'm':
            plotRef(ser, "step", False)
        elif selection == 'n':
            plotRef(ser, "cubic", False)
        elif selection in plotting:
            read_plot_matrix(ser)
        elif selection == 'r':
            # get the mode
            mode = ser.read_until(b'\n')
            # print it to the screen
            print('Now is in mode: ' + modes_dict[int(mode.decode('utf-8'))] + '\n')
        elif (selection == 'x'):
            write_input(ser, "Enter two numbers for addition:")
            get_line(ser)
        elif (selection == 'q'):
            print('Exiting client')
            has_quit = True; # exit client
            # be sure to close the port
            ser.close()
        else:
            print('Invalid Selection ' + selection_endline)


if __name__ == "__main__":
    main()

