# Server_Parallel.py

import socket
import multiprocessing
from multiprocessing.reduction import reduce_handle

import ServerParallelBackStage

if __name__ == "__main__":
    print "Start Back stage"
    queue = multiprocessing.Queue()
    process_back_stage = multiprocessing.\
        Process(target=ServerParallelBackStage.Main, args=(queue,))
    process_back_stage.start()
    # initialization and listening
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    HOST = '114.214.198.136'
    PORT = 1060
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1)
    while True:
        print 'Listening at', s.getsockname()
        sc, sockname = s.accept()
        print 'We have accepted a connection from ', sockname
        print 'Socket connects',\
            sc.getsockname(), 'and', sc.getpeername()
        # Send the socket through queue
        h = reduce_handle(sc.fileno())
        print "put"
        queue.put(h)
