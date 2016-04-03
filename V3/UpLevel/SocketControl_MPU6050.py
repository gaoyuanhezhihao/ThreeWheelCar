# SocketControl_MPU6050.py
'''
This car admin software is used for Tang's car.
The down side is the arduino uno board.
'''
import logging
import time
import socket
import serial
from Calibrate import CarAdmin
from threading import Thread


def recv_all(sock, length):
    data = ""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data


class CarSocketAdmin(CarAdmin):

    def __init__(self, name, ServerIP, ServerPort, ID):
        CarAdmin.__init__(self, name)
        self.ServerIP = ServerIP
        self.ServerPort = ServerPort
        self.ID = ID
        self.GlobalMem = 0
        self.GlobalFlag = 0
        self.RightAckFlag = 0
        Port_name_MPU6050 = raw_input("Choose the MPU6050 port\n")
        self.port_mpu6050 = serial.Serial(Port_name_MPU6050, 9600)
        self.angle = 0
        self.mpu6050_start_angle = 0
        self.turning_angle = 0
        self.Rcv_Buffer = 0
        self.RcvByte = 0
        self.Order_Sock_MPU6050 = 0
        self.angle_over_360 = 0
        self.stop_angle_over_360 = 0
        self.stop_angle_range = [[0,0],[0,0]]
        self.sock_client = 0
        self.pwm_value = 0

    def TouchTheCar(self):
# if time.time() - self.LastAckTime > 1:
# print "Lost Connect\n"
        if time.time() - self.LastAckTime > 0.5:
            self.SendOrder(self.LastSentOrder)

    def SocketClient(self):
        LegalOrder = ['g', 'l', 'r', 'f', 's', 'b', 'p', 'q']
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((SERVERIP, SERVERPORT))
        s.listen(1)
        while True:
            logger.info('Listening at'+str(s.getsockname()))
            # print 'Listening at', s.getsockname()
            self.sock_client, sockname = s.accept()
            logger.info( 'We have accepted a connection from '+str(sockname))
            logger.info('Socket connects'+str(self.sock_client.getsockname())+ 'and'+str(self.sock_client.getpeername()))
            # print 'We have accepted a connection from ', sockname
            # print 'Socket connects', self.sock_client.getsockname(), 'and', self.sock_client.getpeername()
            while True:
                try:
                    message = self.sock_client.recv(1024)
                    self.sock_client.sendall("ok\n")
                    logger.info("socket replyed:"+"ok\n")
                    logger.info("recv"+repr(message))
                    # print "recv", repr(message), '\n'
                    command_tokens = message.split('\n')
                    if command_tokens[0] in ['g', 'f', 's', 'b']:
                        self.GlobalMem = command_tokens[0]
                        self.GlobalFlag = 1
                        self.Order_Sock_MPU6050 = command_tokens[0]
                    elif command_tokens[0] in ['l', 'r'] and len(command_tokens) >= 2:
                        self.Order_Sock_MPU6050 = command_tokens[0]
                        self.turning_angle = int(command_tokens[1])
                    elif command_tokens[0] in ['p', 'q'] and len(command_tokens) >= 2:
                        self.GlobalMem = command_tokens[0]
                        self.pwm_value = int(command_tokens[1])
                        self.GlobalFlag = 1
                except Exception, e:
                    logger.error("*** connection failed."+str(e)+"\n Delete the couple ***")
                    # print "*** connection failed.", e, "\n Delete the couple ***"
                    self.sock_client.shutdown(socket.SHUT_RDWR)
                    self.sock_client.close()
                    break

    def ReadMPU6050(self):
        while True:
            # update the attitude angle
            RcvByte = self.port_mpu6050.read()
            # print "RcvByte:", RcvByte
            if ord(RcvByte) == 0x55:
                Rcv_Buffer = self.port_mpu6050.read(9)
                if ord(Rcv_Buffer[0]) == 0x53:
                    self.angle = (
                        ord(Rcv_Buffer[6]) << 8 | ord(Rcv_Buffer[5])) / 32768.0 * 180
                    # if time() - last_time > 0.5:
                    #     last_time = time()
                    #     print angle, '\n'
            if self.Order_Sock_MPU6050 in ['l', 'r']:
                # Turning order
                self.mpu6050_turing_side = self.Order_Sock_MPU6050
                self.Order_Sock_MPU6050 = 0
                self.mpu6050_start_angle = self.angle
                # if self.mpu6050_turing_side == 'l':
                #     self.mpu6050_stop_angle = self.angle + self.turning_angle
                #     if self.mpu6050_stop_angle > 360:
                #         self.mpu6050_stop_angle -= 360
                #         self.stop_angle_over_360 = 1
                # else:
                #     self.mpu6050_stop_angle = self.angle - self.turning_angle
                self.calculate_stop_range()
                self.GlobalMem = self.mpu6050_turing_side
                self.GlobalFlag = 1
                logger.info("car start turning\n")
                print "car start turning\n"
                while True:
                    # #update the attitude angle
                    RcvByte = self.port_mpu6050.read()
                    # print "RcvByte:", RcvByte
                    if ord(RcvByte) == 0x55:
                        Rcv_Buffer = self.port_mpu6050.read(9)
                        # print ord(Rcv_Buffer[0]), ord(Rcv_Buffer[0]) == 0x53
                        if ord(Rcv_Buffer[0]) == 0x53:
                            self.angle = (ord(Rcv_Buffer[6]) << 8 | ord(Rcv_Buffer[5])) / 32768.0 * 180
                            if self.angle > 360:
                                self.angle -= 360
                                self.angle_over_360 = 1
                    # #Continue turning?
                    if self.check_if_stop():
                        self.GlobalMem = 's'
                        self.GlobalFlag = 1
                        logger.info("car stop turn")
                        print "car stop turn\n"
                        try:
                            self.sock_client.sendall(self.mpu6050_turing_side+"_ok\n")
                        except socket.error, e:
                            logger.error("***ReadMPU6050: connection failed, " + str(e))
                            print "ReadMPU6050: connection failed"
                        break
                    if self.if_order_changed():
                        logger.info("car stop turn by stop order")
                        print "car stop turn by stop order\n"
                        try:
                            self.sock_client.sendall(self.mpu6050_turing_side+"_fail\n")
                        except socket.error, e:
                            logger.error("***ReadMPU6050: connection failed, " + str(e))
                            print "ReadMPU6050: connection failed"
                        break

    def calculate_stop_range(self):
        if self.mpu6050_turing_side == 'l':
            stop_angle_point = self.mpu6050_start_angle + self.turning_angle
            if stop_angle_point > 360:
                stop_angle_point -= 360
            stop_range1_end = stop_angle_point + 20
            if stop_range1_end > 360:
                # there should be 2 stopping range. When the car's position angle
                # is in one of the two range(mostly a little lefter than stop point),
                # the car should stop
                stop_range2_end = stop_range1_end - 360
                self.stop_angle_range[0][0] = stop_angle_point
                self.stop_angle_range[0][1] = 360
                self.stop_angle_range[1][0] = 0
                self.stop_angle_range[1][1] = stop_range2_end
            else:
                #there is only one range.
                self.stop_angle_range[0][0] = stop_angle_point
                self.stop_angle_range[0][1] = stop_range1_end
                self.stop_angle_range[1] = [0,0]
        elif self.mpu6050_turing_side == 'r':
            stop_angle_point = self.mpu6050_start_angle - self.turning_angle
            if stop_angle_point < 0:
                stop_angle_point += 360
            stop_range1_head = stop_angle_point - 20
            if stop_range1_head < 0:
                # there should be 2 stopping range. When the car's position angle
                # is in one of the two range(mostly a little righter than stop point),
                # the car should stop
                stop_range2_head = stop_range1_head + 360
                self.stop_angle_range[0][0] = 0
                self.stop_angle_range[0][1] = stop_angle_point
                self.stop_angle_range[1][0] = stop_range2_head
                self.stop_angle_range[1][1] = 360
            else:
                # there is only one range.
                self.stop_angle_range[0][0] = stop_range1_head
                self.stop_angle_range[0][1] = stop_angle_point
                self.stop_angle_range[1] = [0, 0]
        else:
            raise

    def if_order_changed(self):
        if self.Order_Sock_MPU6050 == 's':
            self.Order_Sock_MPU6050 = 0;
            return True
        return False

    def check_if_stop(self):
        if self.stop_angle_range[0][0] < self.angle < self.stop_angle_range[0][1] or \
            self.stop_angle_range[1][0] < self.angle < self.stop_angle_range[1][1]:
            return True
        else:
            return False

    def Run(self):
        ThreadSocket = Thread(target=self.SocketClient, args=())
        # ThreadSerialRead = Thread(target=self.ReadTheSerial, args=())
        # ThreadSerialRead.start()
        ThreadMPU6050 = Thread(target=self.ReadMPU6050, args=())
        ThreadSocket.start()
        ThreadMPU6050.start()
        while True:
            if self.GlobalFlag == 1:
                self.GlobalFlag = 0
                if self.GlobalMem in ['p', 'q']:
                    self.Send_Direct_Order(order = self.GlobalMem, data1=self.pwm_value/256, data2=self.pwm_value % 256)
                else:
                    self.send_order(self.GlobalMem)
            self.check_last_send()
            # self.TouchTheCar()

    def send_order(self, order):
        if order == 's':
            self.Send_Direct_Order(order='s')
        elif order == 'l':
            self.Send_Direct_Order(order='l')
        elif order == 'r':
            self.Send_Direct_Order(order='r')
        elif order == 'f':
            self.Send_Direct_Order(order='f')
        elif order == 'b':
            self.Send_Direct_Order(order='b')
if __name__ == '__main__':
    SERVERIP = '127.0.0.1'
    SERVERPORT = 8888
    # create logger with 'spam_application'
    logger = logging.getLogger('SocketControl_MPU6050')
    logger.setLevel(logging.DEBUG)
    # create file handler which logs even debug messages
    fh = logging.FileHandler('SocketControl_MPU6050.log')
    fh.setLevel(logging.DEBUG)
    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.ERROR)
    # create formatter and add it to the handlers
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)
    # add the handlers to the logger
    logger.addHandler(fh)
    logger.addHandler(ch)
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
