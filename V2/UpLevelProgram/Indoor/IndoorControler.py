# InnorControler.py
import time
import socket
from ControlGUI import CarAdmin
from threading import Thread


def recv_all(sock, length):
    data = ""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data

class CarSocketAdmin(CarAdmin):
    def __init__(self, name, ServerIP, ServerPort, ID):
        CarAdmin.__init__(self, name)
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

    def ReadTheSerial(self):
        print "ReadTheSerial start"
        while True:
            self.RcvBuffer.append(self.port.read(1))
            if len(self.RcvBuffer) >= 4:
                if ord(self.RcvBuffer[0]) == 0x54:
                    if ord(self.RcvBuffer[1]) + ord(self.RcvBuffer[2]) \
                            == ord(self.RcvBuffer[3]):
                        if self.RcvBuffer[1] != self.LastSentOrder:
                            print "Recv:", self.RcvBuffer[1], "old :", self.LastSentOrder
                            print "Recv Wrong Acknowledge,Resenting..."
                            self.SendOrder(self.LastSentOrder)
                        else:
##                            print "right ack\n"
                            self.RightAckFlag = 1
                            self.LastAckTime = time.time()
                    else:
                        print "Damaged message\n"
                    print "delete 4"
                    del self.RcvBuffer[0:5]
                else:
                    del self.RcvBuffer[0]

    def SocketClient(self):
        LegalOrder = ['g', 'l', 'r', 'f', 's', 'b']
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((SERVERIP, SERVERPORT))
        s.listen(1)
        CaptainDict = {}
        CarDict = {}
        while True:
            print 'Listening at', s.getsockname()
            sc, sockname = s.accept()
            print 'We have accepted a connection from ', sockname
            print 'Socket connects', sc.getsockname(), 'and', sc.getpeername()
            while True:
                try:
                    message = sc.recv(1024)
                    sc.sendall("ok\n")
                    print "recv", repr(message), '\n'
                    command_tokens = message.split('\n')
                    for token in command_tokens:
                        if token in LegalOrder:
                            self.GlobalMem = token
                            self.GlobalFlag = 1
                except Exception, e:
                    print "*** connection failed.", e,"\n Delete the couple ***"
                    sc.shutdown(socket.SHUT_RDWR)
                    sc.close()
                    break

    def Run(self):
        ThreadSocket = Thread(target=self.SocketClient, args=())
        ThreadSerialRead = Thread(target=self.ReadTheSerial, args=())
##        ThreadSerialRead.start()
        ThreadSocket.start()
        while True:
            if self.GlobalFlag == 1:
                self.GlboalFlag = 1
                self.SendOrder(self.GlobalMem)
            self.TouchTheCar()
if __name__ == '__main__':
    SERVERIP = '127.0.0.1'
    SERVERPORT = 8888
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
