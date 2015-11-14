import multiprocessing
import socket
from multiprocessing.reduction import reduce_handle
from multiprocessing.reduction import rebuild_handle
import time


def writer_proc(q):
    print "writer_proc start"
    SERVERIP = "192.168.1.100"
    SERVERPORT = 8090
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((SERVERIP, SERVERPORT))
    print 'Listening at', s.getsockname()
    s.listen(1)
    sc, sockname = s.accept()
    TimeStamp = time.time()
    h = reduce_handle(sc.fileno())
    print "put"
    q.put(h)
    print "Time pass:", time.time() - TimeStamp
    while True:
        pass


def reader_proc(q):
    h = q.get()
    fd = rebuild_handle(h)
    client_socket = socket.fromfd(fd, socket.AF_INET, socket.SOCK_STREAM)
    print client_socket.recv()
    while True:
        pass
if __name__ == "__main__":
    q = multiprocessing.Queue()
    writer = multiprocessing.Process(target=writer_proc, args=(q,))
    writer.start()

    reader = multiprocessing.Process(target=reader_proc, args=(q,))
    reader.start()

    reader.join()
    writer.join()
