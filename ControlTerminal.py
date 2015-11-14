import serial
import string
import random
from threading import Thread
from Queue import Queue
import time
from time import sleep
##from multiprocessing import Process,Value,Array
from Tkinter import *
def onclick():
    pass
def serial_ports():
    """Lists serial ports

    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    ports = ['COM' + str(i + 1) for i in range(256)]
    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
def AskOrder(Order_queue):
    global GlobalOrder
    LegalOrder = ['g','l','r','f','s']
    while True:
        order=raw_input('Your order:')
        if order in LegalOrder :
            Order_queue.put(order)
            GlobalOrder = 1
        else :
            print "Illegal order\n"
def AskOneCommand():
    LegalOrder = ['g','l','r','f','s']
 
    order=raw_input('Your order:')
    if order in LegalOrder :
        Order_queue.put(order)
        GlobalOrder = 1
    else :
        print "Illegal order\n"
def SendOrder(order):
    print "SendOrder:%s\n"%order
    port.write([0x53])
    port.write([order])
    port.write([0x01])
    port.write([ord(order)+1])
    return order
def CarControler(Order_queue,port):
    global GlobalOrder
    LastSentOrder = 's'
    LastAckTime = 0
    root = Tk()
    text = Text(root)
    text.insert(INSERT, "Hello.....")
    text.insert(END, "Bye Bye.....")
    text.pack()

    text.tag_add("here", "1.0", "1.4")
    text.tag_add("start", "1.8", "1.13")
    text.tag_config("here", background="yellow", foreground="blue")
    text.tag_config("start", background="black", foreground="green")
    while True:
        root.update()
##        print "Thread 2"
##        if 1 == GlobalOrder:
##            GlobalOrder = 0
##            order=Order_queue.get()
##            SendOrder(order)
##            LastSentOrder=order
        if port.inWaiting() >= 1:
            RcvBuffer=port.read(1)
            #print RcvBuffer
            if ord(RcvBuffer) == 0x54:
                RcvBuffer = port.read(3)
                if ord(RcvBuffer[0])+ord(RcvBuffer[1])==ord(RcvBuffer[2]):
                    if RcvBuffer[0] != LastSentOrder:
##                        print "Recv Wrong Acknowledge,Resenting..."
                        SendOrder(LastSentOrder)
                    else:
##                        print "right ack\n"
                        LastAckTime = time.time()
                else :
                    pass
##                    print "Damaged message\n"
        if time.time() - LastAckTime > 1:
            pass
##            print "Lost Connect\n"
            
        if time.time() - LastAckTime > 0.5:
            SendOrder(LastSentOrder)           
        
if __name__ == '__main__':
    print("serial port available:\n")
    print(serial_ports())
    sPortChoic=raw_input("Input the port to open\n")
    port=serial.Serial(sPortChoic,9600)
    q=Queue()
    GlobalOrder =0
    t1=Thread(target=AskOrder,args=(q,))
    t2=Thread(target=CarControler,args=(q,port))
    t1.start()
    t2.start()
                

