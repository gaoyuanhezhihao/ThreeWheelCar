#ControlWithSocket.py
import serial
import time
import socket
from ControlGUI import CarAdmin
from threading import Thread
class CarSocketAdmin(CarAdmin):
    def __init__(self,name,ServerIP,ServerPort,ID):
        CarAdmin.__init__(self,name)
        self.ServerIP=ServerIP
        self.ServerPort=ServerPort
        self.ID=ID
        self.GlobalMem=0
        self.GlobalFlag=0
    def TouchTheCar(self):
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
        
    def SocketClient(self):
        LegalOrder = ['g','l','r','f','s','b']
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                s.connect((SERVERIP,SERVERPORT))
                s.sendall(self.name)
                s.sendall("ConnectBoss")
                while True:
                    command = s.recv(1024)
                    if command in LegalOrder:
                        self.GlobalMem = command
                        self.GlobalFlag = 1
            except Exception,e:
                print "***connection failed.\n",e,"\n Retrying...***"
            finally:
                s.close()
    def Run(self):
        ThreadSocket = Thread(target=self.SocketClient,args=())
        ThreadSocket.start()
        while True:
            if self.GlobalFlag == 1:
                self.GlboalFlag = 1
                self.SendOrder(self.GlobalMem)
            self.TouchTheCar()
if __name__ == '__main__':
    SERVERIP = '222.195.92.21'
    SERVERPORT = 1060
    Admin = CarSocketAdmin('CarCar',SERVERIP,SERVERPORT,1)
    Admin.Run()
                
