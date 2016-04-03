import serial
import Tkinter
import time


class CarAdmin():

    def __init__(self, name):
        self.name = name
        self.State = 's'
        self.RcvBuffer = []
        self.LastSentOrder = 's'
        self.LastAckTime = 0
        print("serial port available:\n")
        print(self.serial_ports())
        sPortChoic = raw_input("Input the port to open\n")
        self.port = serial.Serial(sPortChoic, 9600)
        self.SentOrderRecord = 0

    def Forward(self):
        self.SendOrder(0x01)

    def Backward(self):
        self.SendOrder(0x02)

    def Stop(self):
        self.SendOrder(0x03)

    def SendOrder(self, order):
        if self.SentOrderRecord != order:
            self.SentOrderRecord = order
            print "SendOrder:%s\n" % order
        self.port.write([0x55])
        self.port.write([order])
        self.port.write([0x01])
        self.port.write([order + 1])
        self.LastSentOrder = order
        return order

    def serial_ports(self):
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

    def Run(self):
        self.top = Tkinter.Tk()
        self.Button_F = Tkinter.Button(
            self.top, text="Forward", command=self.Forward)
        self.Button_F.pack()
        self.Button_B = Tkinter.Button(
            self.top, text="Back", command=self.Backward)
        self.Button_B.pack()
        self.Button_S = Tkinter.Button(
            self.top, text="Stop", command=self.Stop)
        self.Button_S.pack()
        while True:
            self.top.update()
            bytes_waiting = self.port.inWaiting()
            if bytes_waiting >= 3:
                report = self.port.read(bytes_waiting)
                print [ord(i) for i in report]
            if time.time() - self.LastAckTime > 0.5:
                self.LastAckTime = time.time()
                self.SendOrder(0x04)

if __name__ == '__main__':

    Admin = CarAdmin('Car')
    Admin.Run()
