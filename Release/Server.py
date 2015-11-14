#Server.py
import socket,sys
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
HOST = '127.0.0.1'
PORT = 1060
def recv_all(sock,length):
    data=""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
s.bind((HOST, PORT))
s.listen(1)
CaptainDict = {}
CarDict = {}
while True:
    print 'Listening at', s.getsockname()
    sc,sockname = s.accept()
    print 'We have accepted a connection from ',sockname
    print 'Socket connects',sc.getsockname(),'and',sc.getpeername()
    message = sc.recv(1024)
    print 'The incoming message says',repr(message)
    MsgList = message.split("\n")
    for index,msg in enumerate(MsgList):
        if msg == "Boss":
            sc.sendall("ack")
            print "***Boss on the board\n"
            if index >= len(MsgList) - 1:
                msg = sc.recv(1024)
            else:
                msg = MsgList[index + 1]
            print 'The incoming message says',repr(msg)
            BossClient = sc
            CaptainDict["Boss"] = BossClient
        elif msg == "CarCar":
            print "***Car is ready\n"
            if index >= len(MsgList) - 1:
                msg = sc.recv(1024)
            print 'The incoming message says',repr(msg)
            CarClient = sc
            CarDict["CarCar"] = CarClient
    if "CarCar" in CarDict and "Boss" in CaptainDict:
        print "Captain and Car are connected.Let's go\n"
        while True:
            try:
                message = CaptainDict["Boss"].recv(1024)
                CaptainDict["Boss"].sendall("ack")
                print "recv", message, "from Boss.Sending to Car...\n"
                CarDict["CarCar"].sendall(message)
            except:
                print "*** connection failed.\n Delete the couple ***"
                CaptainDict["Boss"].shutdown(socket.SHUT_RDWR)
                CaptainDict["Boss"].close()
                CarDict["CarCar"].shutdown(socket.SHUT_RDWR)
                CarDict["CarCar"].close()
                del CarDict["CarCar"]
                del CaptainDict["Boss"]
                break


