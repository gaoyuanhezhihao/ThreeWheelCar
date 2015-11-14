from multiprocessing import Process, Value, Array
import time
import serial
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
def CarControler(OrderSign,OrderShare,sPortChoic):

    
    port=serial.Serial('com5',9600)
    LastSentOrder = 's'
    LastAckTime = 0
    while True:
        if 1 == OrderSign:
            OrderSign = 0
            order=OrderShare
            SendOrder(order)
            LastSentOrder=order
        AskOrder(Order_queue)
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
                        pass
##                        print "right ack\n"
                        LastAckTime = time.time()
                else :
                    pass
##                    print "Damaged message\n"
                
        if time.time() - LastAckTime > 1:
            print "Lost Connect\n"
            pass
        if time.time() - LastAckTime > 0.5:
            SendOrder(LastSentOrder)           
    
if __name__ == '__main__':
    print("serial port available:\n")
    print(serial_ports())
    sPortChoic=raw_input("Input the port to open\n")

    
    OrderSign = Value('d', 0)
    OrderShare = Value('c','q')
    p1 = Process(target=CarControler, args=(OrderSign,OrderShare,sPortChoic))
    p1.start()
    LegalOrder = ['g','l','r','f','s']
    while True:
        order=raw_input('Your order:')
        if order in LegalOrder :
            OrderShare = order
            Ordersign = 1
        else :
            print "Illegal order\n"
