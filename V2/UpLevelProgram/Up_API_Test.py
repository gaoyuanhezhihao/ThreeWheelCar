# Server.py
import socket
import sys
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
HOST = '127.0.0.1'
PORT = 8888


def recv_all(sock, length):
    data = ""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(1)
CaptainDict = {}
CarDict = {}
while True:
    print 'Listening at', s.getsockname()
    sc, sockname = s.accept()
    print 'We have accepted a connection from ', sockname
    print 'Socket connects', sc.getsockname(), 'and', sc.getpeername()
    message = sc.recv(1024)
    print repr(message)
    sc.sendall("ok\n")
    while True:
        try:
            message = sc.recv(1024)
            sc.sendall("ok\n")
            print "recv", repr(message), "\n"
        except:
            print "*** connection failed.\n Delete the couple ***"
    #                CaptainDict["Boss"].shutdown(socket.SHUT_RDWR)
            sc.close()
            del sc
            break
