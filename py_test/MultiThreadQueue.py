from Queue import Queue
from threading import Thread
import time
def producer(out_q):
    data=0
    while True:
        #prodeuce data
        data +=1
        time.sleep(1)
        out_q.put(data)
def consumer(in_q):
    while True:
        #Get some data
        data = in_q.get()
        print "consumer get %d from queue"% data

if __name__ == '__main__':
    q=Queue()
    t1=Thread(target=consumer,args=(q,))
    t2=Thread(target=producer,args=(q,))
    t1.start()
    t2.start()
