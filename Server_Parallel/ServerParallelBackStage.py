# ServerParallelBackStage.py
import threading
import time
import Queue  # replace "Queue" to "queue" in python 3
from multiprocessing.reduction import rebuild_handle
import socket
import select


class CoupleThread(threading.Thread):

    def __init__(self, Socket_1, Which_Part):
        threading.Thread.__init__(self)
        self.Time_Stamp = time.time()
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


class UnknowConnectionThread(threading.Thread):

    def __init__(self, Connection, event):
        threading.Thread.__init__(self)
        self.Connection = Connection
        self.account = None
        self.password = None
        self.start_time = 0
        self.elapse_time = 0
        self.event = event


def Verify_unknow_connection(readable, lonely_car, lonely_boss, unknow_Connection_list):
    for sc in readable:
        login_msg = sc.recv(1024)
        print 'The incoming message says', repr(login_msg)
        Type, account, password, mate = login_msg.split("\n")
        if len(MsgList) == 4:
            if "Car" == Type:
                if mate in lonely_boss:

                lonely_car[account] = sc
            elif "Boss" == Type:
                lonely_boss[account] = sc
            else:
                print "bad connection"
                unknow_Connection_list.remove(sc)
                sc.close()



def Main(queue):
    # init
    unknow_Connection_list = []
    lonely_boss = {}
    lonely_car = {}
    Married_boss = {}
    Married_car = {}
    while True:
        # check the queue.
        h = None
        try:
            h = queue.get(block=False)
        except Queue.Empty:
            pass
        if h is not None:
            fd = rebuild_handle(h)
            client_socket = socket.fromfd(
                fd, socket.AF_INET, socket.SOCK_STREAM)
            unknow_Connection_list.append(client_socket)
        # Use select to check the unknow connection
        readable, writeable, exceptional = select.select(
            unknow_Connection_list, [], [], 20)
        if readable:
            Verify_unknow_connection(
                readable, lonely_car, lonely_boss, unknow_Connection_list)
