# chapter 28 in python

# sudo apt-get install python3-pip
# python3 -m pip install pyserial
# sudo apt-get install python3-matplotlib


import matplotlib.pyplot as plt 
from statistics import mean 
def read_plot_matrix(ser):
    n_str = ser.read_until(b'\n');  # get the number of data points to receive
    n_int = int(n_str) # turn it into an int
    print('Data length = ' + str(n_int))
    ref = []
    data = []
    data_received = 0
    while data_received < n_int:
        dat_str = ser.read_until(b'\n');  # get the data as a string, ints seperated by spaces
        dat_f = list(map(float,dat_str.split())) # now the data is a list
        ref.append(dat_f[0])
        data.append(dat_f[1])
        data_received = data_received + 1
    meanzip = zip(ref,data)
    meanlist = []
    for i,j in meanzip:
        meanlist.append(abs(i-j))
    score = mean(meanlist)
    t = range(len(ref)) # index array
    plt.plot(t,ref,'r*-',t,data,'b*-')
    plt.title('Score = ' + str(score))
    plt.ylabel('value')
    plt.xlabel('index')
    plt.show()

