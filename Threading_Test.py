#Threading_Test.py
import time
import threading


class SubThread(threading.Thread):
    def __init__(self, Socket_1, Which_Part):
        threading.Thread.__init__(self)
        self.Time_Stamp =
        if Which_Part == "Car":
            self.CarSocket = Socket_1
            self.CaptainSocket = None
        else:
            self.CaptainSocket = Socket_1
            self.CarSocket = None

    def run(self):
        while self.CarSocket is None or \
                self.CaptainSocket is None:
            pass
        print "Couple Ready:", self.CarSocket,\
            self.CaptainSocket
        while True:
            self.Time_Stamp = time.time()
            time.sleep(1)

    def Add_Another_Socket(self, Another_Socket):
        if self.CaptainSocket is None:
            self.CaptainSocket = Another_Socket
        else:
            self.CarSocket = Another_Socket
        return 0

if __name__ == '__main__':
    mock_socket_1 = 'a'
    mock_socket_2 = 'b'
    SubThread1 = SubThread(mock_socket_1, 'Car')
    SubThread1.start()
    raw_input("Go?")
    SubThread1.Add_Another_Socket(mock_socket_2)
    OldTime = SubThread1.Time_Stamp
    while True:
        if SubThread1.Time_Stamp != OldTime:
            OldTime = SubThread1.Time_Stamp
            print OldTime
