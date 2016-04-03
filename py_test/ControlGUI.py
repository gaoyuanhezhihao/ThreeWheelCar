import serial
import Tkinter
import time

class CarAdmin():
    def __init__(self,name):
        self.name = name
        self.State='s'
        self.RcvBuffer=[]
        self.LastSentOrder='s'
        self.LastAckTime=0
        print("serial port available:\n")
        print(self.serial_ports())
        sPortChoic=raw_input("Input the port to open\n")
        self.port=serial.Serial(sPortChoic,9600)
        self.SentOrderRecord = 0
    def TurnLeft(self):
        self.State='l'
        self.SendOrder('l')
    def TurnRight(self):
        self.state='r'
        self.SendOrder('r')
    def Forward(self):
        self.state='f'
        self.SendOrder('f')
    def Backward(self):
        self.state='b'
        self.SendOrder('b')
    def SendOrder(self,order):
        if self.SentOrderRecord != order:
            self.SentOrderRecord = order
            print "SendOrder:%s\n"%order
            self.port.write([0x53])
            self.port.write([order])
            self.port.write([0x01])
            self.port.write([ord(order)+1])
            self.LastSentOrder=order
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
        self.Button_F=Tkinter.Button(self.top,text="Forward",command=self.Forward)
        self.Button_F.pack()
        self.Button_L=Tkinter.Button(self.top,text="Left",command=self.TurnLeft)
        self.Button_L.pack()
        self.Button_R=Tkinter.Button(self.top,text="Right",command=self.TurnRight)
        self.Button_R.pack()
        self.Button_B=Tkinter.Button(self.top,text="Back",command=self.Backward)
        self.Button_B.pack()
        while True:
            self.top.update()
            if self.port.inWaiting() >= 1:
                self.RcvBuffer=self.port.read(1)
                #print RcvBuffer
                if ord(self.RcvBuffer) == 0x54:
                    self.RcvBuffer = self.port.read(3)
                    if ord(self.RcvBuffer[0])+ord(self.RcvBuffer[1])==ord(self.RcvBuffer[2]):
                        if self.RcvBuffer[0] != self.LastSentOrder:
                            print "Recv Wrong Acknowledge,Resenting..."
                            self.SendOrder(self.LastSentOrder)
                        else:
                            print "right ack\n"
                            self.LastAckTime = time.time()
                    else :
                        pass
                        print "Damaged message\n"
            if time.time() - self.LastAckTime > 1:
                pass
                print "Lost Connect\n"
                
            if time.time() - self.LastAckTime > 0.5:
                self.SendOrder(self.LastSentOrder)    
        
if __name__ == '__main__':

    Admin = CarAdmin('Car')
    Admin.Run()
