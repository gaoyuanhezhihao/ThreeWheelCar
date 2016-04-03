# MPU6050_test.py
"""
Check if the MPU6050 works well.
"""
import serial
import string
import random
from time import time
from time import sleep


def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    ports = ['COM' + str(i + 1) for i in range(256)]
    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


if __name__ == '__main__':
    print("serial port available:\n")
    print(serial_ports())
    sPortChoic = raw_input("Input the port to open\n")
    port = serial.Serial(sPortChoic, 9600, timeout=0.5)
    Rcv_Buffer = []
    RcvByte = 0
    angle = range(3)
    last_time = time()
    while 1:
        port.write([0x31])
        Rcv_Buffer = port.read(8)
        if len(Rcv_Buffer) == 8:
            print Rcv_Buffer[2], Rcv_Buffer[3], Rcv_Buffer[4], '.', Rcv_Buffer[6], '\n'
        sleep(0.5)

