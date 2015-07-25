import serial
import string
import random
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
    sPortChoic=raw_input("Input the port to open\n")
    port=serial.Serial(sPortChoic,9600)
    SendByte = 'h'
    RevByte = '\0'
    GoodCount=0;
    BadCount=0;
    PrintSuccessRate=0;
    while 1:
        sleep(0.01)
        SendByte = random.choice(string.letters)
        port.write(SendByte)
        sleep(0.01)
        RevByte=port.read()
        if SendByte != RevByte:
            print("ERROR!Send:",SendByte,"Receive:",RevByte)
            BadCount+=1;
        else:
            GoodCount+=1;
        if (GoodCount+BadCount) > 1:
            print("success rate:%f"% (float(GoodCount)/(GoodCount+BadCount)),"Good:",GoodCount,"Bad",BadCount)

