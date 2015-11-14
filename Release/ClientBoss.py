# ClientBoss.py
import socket
SERVERIP = "114.214.198.136"
SERVERPORT = 1060
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


while True:
    try:
        s.connect((SERVERIP, SERVERPORT))
        s.sendall("Boss\n")
        s.sendall("ConnectCarCar")
        while True:
            msg = raw_input("Your command:")
            s.sendall(msg)
    except Exception, e:
        print "*** connection failed. \n", e, "\nRetrying"
