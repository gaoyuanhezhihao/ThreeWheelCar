#__author__ = 'Anna Sherlock'
# ControlWithSocket.py
import time
import socket
from ControlGUI import CarAdmin
from threading import Thread

class WrapCarAdmin(CarAdmin):
    def __init__(self,name):
        self.name = name
        self.State='s'
        self.RcvBuffer=[]
        self.LastSentOrder='s'
        self.LastAckTime=0
    def SendOrder(self,order):
        # print "SendOrder:%s\n"%order
        return order

class CarSocketAdmin(WrapCarAdmin):
    def __init__(self, name, ServerIP, ServerPort, ID):
        WrapCarAdmin.__init__(self, name)
        self.ServerIP = ServerIP
        self.ServerPort = ServerPort
        self.ID = ID
        self.GlobalMem = 0
        self.GlobalFlag = 0
        self.RightAckFlag = 0

    def TouchTheCar(self):

##        if time.time() - self.LastAckTime > 1:
##            print "Lost Connect\n"

        if time.time() - self.LastAckTime > 0.5:
            self.SendOrder(self.LastSentOrder)


#     def ReadTheSerial(self):
#         print "ReadTheSerial start"
#         while True:
#             self.RcvBuffer.append(self.port.read(1))
#             if len(self.RcvBuffer) >= 4:
#                 if ord(self.RcvBuffer[0]) == 0x54:
#                     if ord(self.RcvBuffer[1]) + ord(self.RcvBuffer[2]) \
#                             == ord(self.RcvBuffer[3]):
#                         if self.RcvBuffer[1] != self.LastSentOrder:
#                             print "Recv:", self.RcvBuffer[1], "old :", self.LastSentOrder
#                             print "Recv Wrong Acknowledge,Resenting..."
#                             self.SendOrder(self.LastSentOrder)
#                         else:
# ##                            print "right ack\n"
#                             self.RightAckFlag = 1
#                             self.LastAckTime = time.time()
#                     else:
#                         print "Damaged message\n"
#                     print "delete 4"
#                     del self.RcvBuffer[0:5]
#                 else:
#                     del self.RcvBuffer[0]

    def SocketClient(self):

        LegalOrder = ['g', 'l', 'r', 'f', 's', 'b']
        while True:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                print "connect to server"
                s.connect((SERVERIP, SERVERPORT))
                s.sendall(self.name+'\n')
                s.sendall("ConnectBoss"+'\n')
                while True:
                    command = s.recv(1024)
                    print "Recv:", repr(command)
                    if command in LegalOrder:
                        self.GlobalMem = command
                        self.GlobalFlag = 1
                    elif command == '':
                        print "***connection failed"
                        raise
            except Exception, e:
                s.close()
                print "***connection failed.\n", e, "\n Retrying...***"


    def Run(self):
        ThreadSocket = Thread(target=self.SocketClient, args=())
        # ThreadSerialRead = Thread(target=self.ReadTheSerial, args=())
##        ThreadSerialRead.start()
        ThreadSocket.start()
        while True:
            if self.GlobalFlag == 1:
                self.GlboalFlag = 1
                self.SendOrder(self.GlobalMem)
            self.TouchTheCar()
if __name__ == '__main__':
    SERVERIP = '127.0.0.1'
    SERVERPORT = 1060
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
