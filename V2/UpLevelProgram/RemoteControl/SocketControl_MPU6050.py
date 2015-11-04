# SocketControl_MPU6050.py
import time
import socket
import serial
from ControlGUI import CarAdmin
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
        self.port_mpu6050 = serial.Serial(Port_name_MPU6050, 115200)
        self.angle = 0
        self.mpu6050_start_angle = 0
        self.turning_angle = 0
        self.Rcv_Buffer = 0
        self.RcvByte = 0
        self.Order_Sock_MPU6050 = 0
        self.angle_over_360 = 0
        self.stop_angle_over_360 = 0
        self.stop_angle_range = [[0,0],[0,0]]

    def TouchTheCar(self):
# if time.time() - self.LastAckTime > 1:
# print "Lost Connect\n"
        if time.time() - self.LastAckTime > 0.5:
            self.SendOrder(self.LastSentOrder)

#     def ReadTheSerial(self):
#         print "ReadTheSerial start"
#         while True:
#             self.RcvBuffer.append(self.port.read(1))
#             if len(self.RcvBuffer) >= 4:
#                 if ord(self.RcvBuffer[0]) == 0x54:
#                     if ord(self.RcvBuffer[1]) + ord(self.RcvBuffer[2]) \
#                             == ord(self.RcvBuffer[3]):
#                         if self.RcvBuffer[1] != self.LastSentOrder:
#                             print "Recv:", self.RcvBuffer[1], "old :", self.LastSentOrder
#                             print "Recv Wrong Acknowledge,Resenting..."
#                             self.SendOrder(self.LastSentOrder)
#                         else:
# print "right ack\n"
#                             self.RightAckFlag = 1
#                             self.LastAckTime = time.time()
#                     else:
#                         print "Damaged message\n"
#                     print "delete 4"
#                     del self.RcvBuffer[0:5]
#                 else:
#                     del self.RcvBuffer[0]

    def SocketClient(self):
        LegalOrder = ['g', 'l', 'r', 'f', 's', 'b']
        while True:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            try:
                # print "connect to server"
                s.connect((SERVERIP, SERVERPORT))
                s.sendall(self.name + '\n')
                s.sendall("ConnectBoss" + '\n')
                while True:
                    command = s.recv(1024)
                    if command == "KeepAlive\n":
                        continue
                    if command == '':
                        # print "***connection failed"
                        raise
                    # print "Recv:", repr(command)
                    command_tokens = command.split('\n')
                    if command_tokens[0] in ['g', 'f', 's', 'b']:
                        self.GlobalMem = command_tokens[0]
                        self.GlobalFlag = 1
                        self.Order_Sock_MPU6050 = command_tokens[0]
                    elif command_tokens[0] in ['l', 'r'] and len(command_tokens) >= 2:
                        self.Order_Sock_MPU6050 = command_tokens[0]
                        self.turning_angle = int(command_tokens[1])

            except Exception, e:
                self.GlobalMem = 's'
                s.close()
                # print "***connection failed.\n", e, "\n Retrying...***"

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
                        print "car stop turn\n"
                        break
                    if self.if_order_changed():
                        print "car stop turn by order changed\n"
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
                self.stop_angle_range[1] = [0,0]
        else:
            raise

    def if_order_changed(self):
        if self.Order_Sock_MPU6050 != 0:
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
                self.GlboalFlag = 0
                self.SendOrder(self.GlobalMem)
            self.TouchTheCar()
if __name__ == '__main__':
    SERVERIP = '114.214.166.205'
    SERVERPORT = 1060
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
