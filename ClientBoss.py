#ClientBoss.py
import socket
SERVERIP = "114.214.198.136"
SERVERPORT = 1060
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((SERVERIP,SERVERPORT))


try:
    s.send("Boss")
    s.send("ConnectCarCar")
    while True:
        msg = raw_input("Your command:")
        s.send(msg)
finally:
    s.close()
