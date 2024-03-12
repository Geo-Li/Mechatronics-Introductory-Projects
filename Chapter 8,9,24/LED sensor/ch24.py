# chapter 24 in python

# requires pyserial
# requires matplotlib

import serial
# your com port name
ser = serial.Serial('/dev/tty.usbserial-10',230400)
print('Opening port: ')
print(ser.name)

# send 2 gains
ser.write(b'.1 .1\n') # Kp Ki

# read the data back as int in reverse order
sampnum = 0
read_samples = 10
ADCval = []
ref = []
oc = []
eint = []
while read_samples > 1:
    data_read = ser.read_until(b'\n',50)
    data_text = str(data_read,'utf-8')
    data = list(map(int,data_text.split()))

    if(len(data)==3):
        read_samples = data[0]
        ADCval.append(data[1])
        ref.append(data[2])
        sampnum = sampnum + 1

# plot it
import matplotlib.pyplot as plt 
t = range(len(ADCval)) # time array
plt.plot(t,ADCval,'r*-',t,ref,'b*-')
plt.ylabel('value')
plt.xlabel('sample')
plt.show()

# be sure to close the port
ser.close()