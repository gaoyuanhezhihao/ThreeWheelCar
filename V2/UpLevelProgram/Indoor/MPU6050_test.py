# MPU6050_test.py
"""
Check if the MPU6050 works well.
"""
import serial
import string
import random
from time import time


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
    port = serial.Serial(sPortChoic, 115200)
    Rcv_Buffer = []
    RcvByte = 0
    angle = range(3)
    last_time = time()
    while 1:
        RcvByte = port.read()
        # print "RcvByte:", RcvByte
        if ord(RcvByte) == 0x55:
            Rcv_Buffer = port.read(9)
            if ord(Rcv_Buffer[0]) == 0x53:
                angle[0] = (ord(Rcv_Buffer[2]) << 8 | ord(Rcv_Buffer[1]))/32768.0*180
                angle[1] = (ord(Rcv_Buffer[4]) << 8 | ord(Rcv_Buffer[3]))/32768.0*180
                angle[2] = (ord(Rcv_Buffer[6]) << 8 | ord(Rcv_Buffer[5]))/32768.0*180
                if time() - last_time > 0.5:
                    last_time = time()
                    print angle, '\n'

