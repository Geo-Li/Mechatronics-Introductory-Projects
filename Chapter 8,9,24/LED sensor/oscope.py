# plot adc data from pic in python

# requires pyserial
# requires matplotlib

import serial
ser = serial.Serial('/dev/tty.usbserial-10',230400)
print('Opening port: ')
print(ser.name)

# request 100 data points
ser.write(b'b 100\n')

# data is printed back in reverse, index and value, as int
sampnum = 0
read_samples = 10
ADCval = []
while read_samples > 1:
    data_read = ser.read_until(b'\n',50)
    data_text = str(data_read,'utf-8')
    data = list(map(int,data_text.split()))

    if(len(data)==2):
        read_samples = data[0]
        ADCval.append(data[1]*3.3/1024)
        sampnum = sampnum + 1

# plot it
import matplotlib.pyplot as plt 
t = range(len(ADCval)) # time array
plt.plot(t,ADCval,'r*-')
plt.ylabel('value')
plt.xlabel('sample')
plt.show()

# be sure to close the port
ser.close()