# Up_level_API_test_client.py
import socket
SERVERIP = "127.0.0.1"
SERVERPORT = 8888
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((SERVERIP,SERVERPORT))


try:
    while True:
        msg = raw_input("Your command:")
        s.send(msg)
        print s.recv(1024)
finally:
    s.close()
